/*
 * @Author: BigMAN
 * @Date: 2026-08-29 18:16:14
 * @LastEditors: BigMAN
 * @LastEditTime: 2026-09-20 11:24:45
 * @Contact: 13067060853@163.com
 * @Description: 灵巧手控制
 * SPDX-License-Identifier: MIT
 */

#ifndef __CONTROL_H
#define __CONTROL_H

#ifdef __cplusplus
extern "C"
{
#endif

#define DEBUG_LOG 1

#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "usart.h"

    typedef struct
    {
        UART_HandleTypeDef *huart;
        I2C_HandleTypeDef *hi2c;
        ADC_HandleTypeDef *hadc;
    } control_t;

    /**
     * @description: hand ctrol init
     * @param [control_t] handle
     * @return [*]1:fail 0:successful
     */
    uint8_t bll_control_init(control_t handle);

    /**
     * @description: hand control
     * @param [uint8_t] *buffer
     * @return [*]1:fail 0:successful
     */
    void bll_control_hand(uint8_t *buffer, uint16_t buf_len);

#ifdef __cplusplus
}
#endif

#endif /* __CONTROL_H */
