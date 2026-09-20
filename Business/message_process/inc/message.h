/*
 * @Author: BigMAN
 * @Date: 2026-08-29 15:34:54
 * @LastEditors: BigMAN
 * @LastEditTime: 2026-08-29 16:18:03
 * @Contact: 13067060853@163.com
 * @Description: 接收串口数据处理
 * SPDX-License-Identifier: MIT
 */

#ifndef __MESSAGE_H
#define __MESSAGE_H

#include "main.h"
#include "usart.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @description: Message init
     * @param [UART_HandleTypeDef] *huart
     * @return [*]1:fail 0:successful
     */
    uint8_t bll_message_init(UART_HandleTypeDef *huart);

    /**
     * @description: Message get
     * @return [*]1:fail 0:successful
     */
    void bll_message_get(void);

    /**
     * @description: Message relase (buffer size must >= 64byte)
     * @param [char] *buffer
     * @return [*]1:fail 0:successful
     */
    uint8_t bll_message_relase(char *buffer, uint16_t buf_len);

#ifdef __cplusplus
}
#endif

#endif /* __MESSAGE_H*/
