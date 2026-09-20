/*
 * @Author: BigMAN
 * @Date: 2026-08-29 18:15:57
 * @LastEditors: BigMAN
 * @LastEditTime: 2026-09-20 14:08:07
 * @Contact: 13067060853@163.com
 * @Description: 灵巧手控制
 * SPDX-License-Identifier: MIT
 */
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "control.h"
#include "pca9685.h"
#include "FTregister.h"

/* Variables ------------------------------------------------------------------*/
#define CMD_PING 0X01
#define CMD_READ 0X02
#define CMD_WRITE 0X03
#define CMD_SYNC_WRITE 0x83

typedef struct
{
    uint8_t id;
    uint8_t length;
    uint8_t cmd;
} protocol_t;

static control_t s_handle;
static protocol_t s_protocol;

/* Static function ------------------------------------------------------------------*/
static uint8_t s_get_servo(uint8_t servo_id)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    uint32_t ADC_Channel;
    switch (servo_id)
    {
    case 1:
        ADC_Channel = ADC_CHANNEL_0;
        break;
    case 2:
        ADC_Channel = ADC_CHANNEL_1;
        break;
    case 3:
        ADC_Channel = ADC_CHANNEL_2;
        break;
    case 4:
        ADC_Channel = ADC_CHANNEL_3;
        break;
    case 5:
        ADC_Channel = ADC_CHANNEL_4;
        break;
    case 6:
        ADC_Channel = ADC_CHANNEL_5;
        break;
    case 7:
        ADC_Channel = ADC_CHANNEL_6;
        break;
    case 8:
        ADC_Channel = ADC_CHANNEL_7;
        break;
    case 9:
        ADC_Channel = ADC_CHANNEL_8;
        break;
    case 10:
        ADC_Channel = ADC_CHANNEL_9;
        break;
    default:
        return 1;
    }
    sConfig.Channel = ADC_Channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(s_handle.hadc, &sConfig) != HAL_OK)
    {
        return 1;
    }

    HAL_ADC_Start(s_handle.hadc);
    if (HAL_ADC_PollForConversion(s_handle.hadc, 10) == HAL_OK)
    {
        (void)HAL_ADC_GetValue(s_handle.hadc);
    }
    HAL_ADC_Stop(s_handle.hadc);

    uint32_t sum = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        HAL_ADC_Start(s_handle.hadc);
        if (HAL_ADC_PollForConversion(s_handle.hadc, 10) != HAL_OK)
        {
            HAL_ADC_Stop(s_handle.hadc);
            return 1;
        }
        sum += HAL_ADC_GetValue(s_handle.hadc);
        HAL_ADC_Stop(s_handle.hadc);
    }
    uint16_t adc_val = sum / 8;
    uint16_t adc_mv = (uint32_t)adc_val * 3300U / 4095U;
#if 1
    printf("Servo %d: %d mv  Val: %d\n", servo_id, adc_mv, adc_val);
#endif
    if (adc_mv >= 8 && adc_mv <= 250)
        return 0;
    else
        return 1;
}

static uint8_t s_servo_control(uint8_t id, uint8_t addr, uint8_t *buffer, uint8_t len)
{
    static uint16_t s_last_angle[11] = {0};
    uint16_t ms_per_deg = 0;

    bll_write_register(id, addr, &buffer[6], len);

    if (addr <= GOAL_POS_REG_H && addr >= SRAM_START_VOID0)
    {
        uint8_t pos_L = 6 + GOAL_POS_REG_L - addr;
        uint8_t pos_H = 6 + GOAL_POS_REG_H - addr;
        uint8_t speed_L = 6 + GOAL_SPEED_REG_L - addr;
        uint8_t speed_H = 6 + GOAL_SPEED_REG_H - addr;
        uint8_t time_L = 6 + GOAL_TIME_REG_L - addr;
        uint8_t time_H = 6 + GOAL_TIME_REG_H - addr;

        uint16_t speed = (uint16_t)(buffer[speed_L] << 8) | buffer[speed_H];
        uint16_t pos = (uint16_t)(buffer[pos_L] << 8) | buffer[pos_H];
        uint16_t time = (uint16_t)(buffer[time_L] << 8) | buffer[time_H];

        uint8_t target_angle = 180 - (uint32_t)pos * 180 / 1023;
        uint8_t cur_angle = (uint8_t)s_last_angle[id];
        int16_t delta = (int16_t)target_angle - (int16_t)cur_angle;

        if (delta == 0)
        {
            __nop();
        }
        else if (speed == 0)
        {
            fml_servo_set_angle(id, target_angle);
        }
        else if (time > 0)
        {
            uint16_t total_steps = (delta > 0) ? delta : -delta;
            uint16_t ms_per_step = time / total_steps;
            if (ms_per_step == 0)
                ms_per_step = 1;

            int8_t dir = (delta > 0) ? 1 : -1;
            int16_t step = 0;
            while (step != delta)
            {
                step += dir;
                fml_servo_set_angle(id, (uint8_t)(cur_angle + step));

                HAL_Delay(ms_per_step);
            }
        }
        else if (speed > 0)
        {
            ms_per_deg = (uint16_t)(1666U / speed);
            if (ms_per_deg == 0U)
                ms_per_deg = 1U;

            int8_t dir = (delta > 0) ? 1 : -1;
            int16_t step = 0;

            while (step != delta)
            {
                step += dir;
                fml_servo_set_angle(id, (uint8_t)(cur_angle + step));

                HAL_Delay(ms_per_deg);
            }
        }
        s_last_angle[id] = target_angle;

        bll_write_register(id, CURRENT_POS_REG_L, &buffer[pos_L], 1);
        bll_write_register(id, CURRENT_POS_REG_H, &buffer[pos_H], 1);
        bll_write_register(id, CURRENT_SPEED_REG_L, &buffer[speed_L], 1);
        bll_write_register(id, CURRENT_SPEED_REG_H, &buffer[speed_H], 1);

#if DEBUG_LOG
        printf("Servo %d pos=%d, angle=%d, speed=%d, time=%d\r\n", id, pos, target_angle, speed, time);
#endif
        return 0;
    }
    else
        return 1;
}

/* Public function ------------------------------------------------------------------*/
/**
 * @description: hand ctrol init
 * @param [control_t] handle
 * @return [*]1:fail 0:successful
 */
uint8_t bll_control_init(control_t handle)
{
    s_handle.huart = handle.huart;
    s_handle.hi2c = handle.hi2c;
    s_handle.hadc = handle.hadc;

    HAL_ADCEx_Calibration_Start(s_handle.hadc);

    bll_register_init();

    if (fml_servo_init(s_handle.hi2c) == 1)
        return 1;

    return 0;
}

/**
 * @description: Hand control
 * @param [uint8_t] *buffer
 * @param [uint16_t] buf_len 接收缓冲区有效长度，修复原sizeof(buffer)指针bug
 * @return [*]1:fail 0:successful
 */
void bll_control_hand(uint8_t *buffer, uint16_t buf_len)
{
    s_protocol.id = buffer[2];
    s_protocol.length = buffer[3];
    s_protocol.cmd = buffer[4];

    switch (s_protocol.cmd)
    {
        /**
         * Handling the 0x01 ping command: Display it exactly as received.
         */
    case CMD_PING:
    {
        if (s_get_servo(s_protocol.id) != 0)
            return;

        HAL_UART_Transmit(s_handle.huart, buffer, buf_len, 100);
        break;
    }

        /**
         * Handling the 0x02 read command: Read the EEPROM and display it.
         */
    case CMD_READ:
    {
        if (s_get_servo(s_protocol.id) != 0)
            return;

        if (s_protocol.length == 0x04)
        {
            uint8_t response[32];
            uint8_t addr = buffer[5];
            uint8_t len = buffer[6];
            uint8_t real_len = 0;

            response[0] = 0xFF;
            response[1] = 0xFF;
            response[2] = s_protocol.id;
            response[4] = 0x00;

            real_len = bll_read_register(s_protocol.id, addr, &response[5], len);
            if (real_len == 0)
                return;
            response[3] = real_len + 2;

            uint16_t sum = 0;
            for (uint8_t i = 2; i < 5 + real_len; i++)
            {
                sum += response[i];
            }
            response[5 + real_len] = (uint8_t)(~(sum & 0xFF));
#if DEBUG_LOG
            printf("0x02 Client : ");
            for (uint8_t i = 0; i < 5 + real_len + 1; i++)
            {
                printf("%02X ", response[i]);
            }
            printf("\n");
#endif
            HAL_UART_Transmit(s_handle.huart, response, 5 + real_len + 1, 100);
        }
        break;
    }

    /**
     * Handling the 0x03 write command: Write to the EEPROM.
     */
    case CMD_WRITE:
    {
        uint8_t addr = buffer[5];
        uint8_t len = s_protocol.length - 3;

        /* 0xFE broadcast*/
        if (s_protocol.id == 0xFE)
        {
            for (uint8_t id = 1; id <= 10; id++)
            {
                s_servo_control(id, addr, buffer, len);
            }
        }
        else /* unicast*/
        {
            uint8_t response[6];
            uint16_t sum = 0;

            s_servo_control(s_protocol.id, addr, buffer, len);

            response[0] = 0xFF;
            response[1] = 0xFF;
            response[2] = s_protocol.id;
            response[3] = 0x02;
            response[4] = 0x00;

            for (uint8_t i = 2; i < 5; i++)
            {
                sum += response[i];
            }
            response[5] = (uint8_t)(~(sum & 0xFF));
#if DEBUG_LOG
            printf("0x03 Client : ");
            for (uint8_t i = 0; i < 6; i++)
            {
                printf("%02X ", response[i]);
            }
            printf("\n");
#endif
            HAL_UART_Transmit(s_handle.huart, response, 6, 100);
        }
        break;
    }

    /**
     * Handling the 0x83 sync write command.
     */
    case CMD_SYNC_WRITE:
    {
        uint8_t addr = buffer[5];
        uint8_t len = buffer[6];
        uint8_t temp[32] = {0};
        uint16_t idx = 7U;

        uint8_t count = (uint8_t)((s_protocol.length - 4U) / (1U + len));
        for (uint8_t i = 0; i < count; i++)
        {
            uint8_t id = buffer[idx++];

            memset(temp, 0, sizeof(temp));
            memcpy(&temp[6], &buffer[idx], len);

            s_servo_control(id, addr, temp, len);

            idx += len;
        }
        break;
    }
    }
}
