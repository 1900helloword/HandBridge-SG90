/*
 * @Author: BigMAN
 * @Date: 2026-08-29 09:47:35
 * @LastEditors: BigMAN
 * @LastEditTime: 2026-08-29 10:52:45
 * @Contact: 13067060853@163.com
 * @Description: 该舵机驱动仅适用裸机+SG90舵机
 * SPDX-License-Identifier: MIT
 */

#ifndef __PCA9685_H
#define __PCA9685_H

#include "main.h"
#include "i2c.h"

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @description: Servo initialization
     * @param [I2C_HandleTypeDef] *hi2c
     * @return [*]1:fail 0:successful
     */
    uint8_t fml_servo_init(I2C_HandleTypeDef *hi2c);

    /**
     * @description: Serovo frequency set
     * @param [uint16_t] freq
     * @return [*]1:fail 0:successful
     */
    uint8_t fml_servo_set_freq(uint16_t freq);

    /**
     * @description: Servo pwm set
     * @param [uint8_t] channel
     * @param [uint16_t] on
     * @param [uint16_t] off
     * @return [*]
     */
    uint8_t fml_servo_set_pwm(uint8_t channel, uint16_t on, uint16_t off);

    /**
     * @description: Servo angle set
     * @param [uint8_t] ch
     * @param [uint16_t] angle
     * @return [*]1:fail 0:successful
     */
    uint8_t fml_servo_set_angle(uint8_t ch, uint16_t angle);

    /**
     * @description: Servo set raw position value (PWM off count)
     * @param [uint8_t] ch channel
     * @param [uint16_t] pos raw pwm count value(80~550), will add midoffset auto
     * @return 1:fail 0:success
     */
    uint8_t fml_servo_set_pos(uint8_t ch, uint16_t pos);

    /**
     * @description: Servo release
     * @param [uint8_t] ch
     * @return [*]1:fail 0:successful
     */
    uint8_t fml_servo_release(uint8_t ch);

    /**
     * @description: Serovo midoffset set
     * @param [int16_t] offset
     * @return [*]middile offset
     */
    void fml_servo_set_midoffset(int16_t offset);

    /**
     * @description: Servo midoffset get
     * @return [*]middile offset
     */
    int16_t fml_servo_get_midoffset(void);

#ifdef __cplusplus
}
#endif

#endif /* __PCA9685_H */
