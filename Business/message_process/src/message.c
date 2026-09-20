/*
 * @Author: BigMAN
 * @Date: 2026-08-29 15:34:37
 * @LastEditors: BigMAN
 * @LastEditTime: 2026-08-29 20:42:35
 * @Contact: 13067060853@163.com
 * @Description: 接收串口数据处理
 * SPDX-License-Identifier: MIT
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "circular_buffer.h"
#include "message.h"

/* Variables ------------------------------------------------------------------*/
#define FRAME_SIZE 64
#define CIRCULAR_SIZE 256

typedef enum
{
    STATE_HEADER1,
    STATE_HEADER2,
    STATE_ID,
    STATE_LEN,
    STATE_CMD_ERR,
    STATE_PARAM,
    STATE_CHECKSUM,
} ParserState_t;

static UART_HandleTypeDef *s_huart = NULL;
static ParserState_t s_state = STATE_HEADER1;
static uint8_t s_all[FRAME_SIZE] = {0};
static uint8_t s_one = 0;
static uint32_t s_index = 0;
static uint8_t s_len = 0;
static uint8_t s_check = 0;

static circular_buffer s_cb;

/* Public function ------------------------------------------------------------------*/
/**
 * @description: Message init
 * @param [UART_HandleTypeDef] *huart
 * @return [*]1:fail 0:successful
 */
uint8_t bll_message_init(UART_HandleTypeDef *huart)
{
    s_huart = huart;

    if (!circular_buffer_init(&s_cb, CIRCULAR_SIZE))
        return 1;

    HAL_UART_Receive_IT(s_huart, &s_one, 1);
    return 0;
}

/**
 * @description: Message get
 * @return [*]1:fail 0:successful
 */
void bll_message_get(void)
{
    switch (s_state)
    {
    case STATE_HEADER1:
        if (s_one == 0xFF)
        {
            s_check = 0;
            s_index = 0;
            memset(s_all, 0, FRAME_SIZE);

            s_all[s_index++] = s_one;
            s_state = STATE_HEADER2;
        }
        break;

    case STATE_HEADER2:
        if (s_one == 0xFF)
        {
            s_all[s_index++] = s_one;
            s_state = STATE_ID;
        }
        else
        {
            s_state = STATE_HEADER1;
        }
        break;

    case STATE_ID:
        /* just ID1 ~ ID10*/
        if ((s_one >= 1 && s_one <= 10) || s_one == 0xFE)
        {
            s_all[s_index++] = s_one;
            s_check += s_one;
            s_state = STATE_LEN;
        }
        else
        {
            s_state = STATE_HEADER1;
        }
        break;

    case STATE_LEN:
        s_all[s_index++] = s_one;
        s_check += s_one;

        if (s_one > 2)
        {
            s_len = s_one - 2;
            s_state = STATE_CMD_ERR;
        }
        else if (s_one == 2)
        {
            s_len = 0;
            s_state = STATE_CMD_ERR;
        }
        else
        {
            s_state = STATE_HEADER1;
        }
        break;

    case STATE_CMD_ERR:
        s_all[s_index++] = s_one;
        s_check += s_one;

        if (s_len > 0)
        {
            s_state = STATE_PARAM;
        }
        else
        {
            s_state = STATE_CHECKSUM;
        }
        break;

    case STATE_PARAM:
        s_all[s_index++] = s_one;
        s_check += s_one;
        s_len--;
        if (s_len == 0)
        {
            s_state = STATE_CHECKSUM;
        }
        break;

    case STATE_CHECKSUM:
        s_all[s_index++] = s_one;
        uint8_t expected = (uint8_t)(~(s_check & 0xFF));

        if (expected == s_one)
        {
            if (circular_buffer_write(&s_cb, (char *)s_all, FRAME_SIZE) == false)
            {
                printf("write false %d\r\n", FRAME_SIZE);
            }
        }

        s_state = STATE_HEADER1;
        break;
    }

    s_one = 0;
    HAL_UART_Receive_IT(s_huart, &s_one, 1);
}

/**
 * @description: Message relase (buffer size must >= 64byte)
 * @param [char] *buffer
 * @return [*]1:fail 0:successful
 */
uint8_t bll_message_relase(char *buffer, uint16_t buf_len)
{
    if (circular_buffer_read(&s_cb, buffer, (size_t)buf_len) == true)
        return 0;
    else
        return 1;
}
