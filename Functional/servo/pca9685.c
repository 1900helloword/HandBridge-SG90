/*
 * @Author: BigMAN
 * @Date: 2026-08-29 09:47:35
 * @LastEditors: BigMAN
 * @LastEditTime: 2026-08-29 10:48:58
 * @Contact: 13067060853@163.com
 * @Description: 该舵机驱动仅适用裸机+SG90舵机
 * SPDX-License-Identifier: MIT
 */

/* Includes ------------------------------------------------------------------*/
#include "pca9685.h"

/* Variables ------------------------------------------------------------------*/
#define PCA9685_I2C_ADDR 0x40
#define PCA9685_CHANNEL_NUM 16

#define PCA9685_MODE1 0x00
#define PCA9685_MODE2 0x01
#define PCA9685_LED0_ON_L 0x06
#define PCA9685_LED0_ON_H 0x07
#define PCA9685_LED0_OFF_L 0x08
#define PCA9685_LED0_OFF_H 0x09
#define PCA9685_ALL_LED_ON_L 0xFA
#define PCA9685_ALL_LED_ON_H 0xFB
#define PCA9685_ALL_LED_OFF_L 0xFC
#define PCA9685_ALL_LED_OFF_H 0xFD
#define PCA9685_PRE_SCALE 0xFE

#define PCA9685_MODE1_AI (1 << 5)
#define PCA9685_MODE1_SLEEP (1 << 4)
#define PCA9685_MODE1_RESTART (1 << 7)
#define PCA9685_MODE2_OUTDRV (1 << 2)
#define PCA9685_MODE2_OCH (1 << 3)

#define SERVO_PULSE_OFF_0DEG 102
#define SERVO_PULSE_OFF_180DEG 512

static I2C_HandleTypeDef *s_i2c = NULL;
static uint8_t s_init = 0;
static int16_t s_midoffset = 0;

/* Static function ------------------------------------------------------------------*/
static uint8_t s_write_byte(uint8_t reg, uint8_t data)
{
    if (s_i2c == NULL)
        return 1;
    if (HAL_I2C_Mem_Write(s_i2c, (PCA9685_I2C_ADDR << 1), reg,
                          I2C_MEMADD_SIZE_8BIT, &data, 1, 100) != HAL_OK)
        return 1;
    return 0;
}

static uint8_t s_write_string(uint8_t reg, uint8_t *data, uint8_t len)
{
    if (s_i2c == NULL)
        return 1;
    if (HAL_I2C_Mem_Write(s_i2c, (PCA9685_I2C_ADDR << 1), reg,
                          I2C_MEMADD_SIZE_8BIT, data, len, 100) != HAL_OK)
        return 1;
    return 0;
}

static uint8_t s_read_byte(uint8_t reg, uint8_t *pdata)
{
    if (s_i2c == NULL)
        return 1;
    if (HAL_I2C_Mem_Read(s_i2c, (PCA9685_I2C_ADDR << 1), reg,
                         I2C_MEMADD_SIZE_8BIT, pdata, 1, 100) != HAL_OK)
        return 1;
    return 0;
}

/* Public function ------------------------------------------------------------------*/
/**
 * @description: Servo initialization
 * @param [I2C_HandleTypeDef] *hi2c
 * @return [*]1:fail 0:successful
 */
uint8_t fml_servo_init(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL)
        return 1;
    s_i2c = hi2c;

    uint8_t mode1;
    if (s_read_byte(PCA9685_MODE1, &mode1) != 0)
        return 1;

    uint8_t mode2_val = PCA9685_MODE2_OUTDRV | PCA9685_MODE2_OCH;
    if (s_write_byte(PCA9685_MODE2, mode2_val) != 0)
        return 1;

    mode1 |= PCA9685_MODE1_AI;
    mode1 &= ~PCA9685_MODE1_SLEEP;
    mode1 &= ~PCA9685_MODE1_RESTART;
    if (s_write_byte(PCA9685_MODE1, mode1) != 0)
        return 1;

    HAL_Delay(10);
    s_init = 1;

    fml_servo_set_freq(50);
    return 0;
}

/**
 * @description: Serovo frequency set
 * @param [uint16_t] freq
 * @return [*]1:fail 0:successful
 */
uint8_t fml_servo_set_freq(uint16_t freq)
{
    if (!s_init)
        return 1;
    if (freq < 24 || freq > 1526)
        return 1;

    uint32_t prescale = 25000000UL / (4096UL * freq) - 1UL;
    if (prescale > 0xFF)
        prescale = 0xFF;
    if (prescale < 3)
        prescale = 3;

    uint8_t mode1;
    s_read_byte(PCA9685_MODE1, &mode1);

    mode1 |= PCA9685_MODE1_SLEEP;
    s_write_byte(PCA9685_MODE1, mode1);

    s_write_byte(PCA9685_PRE_SCALE, (uint8_t)prescale);

    mode1 &= ~PCA9685_MODE1_SLEEP;
    s_write_byte(PCA9685_MODE1, mode1);

    HAL_Delay(5);
    return 0;
}

/**
 * @description: Servo pwm set
 * @param [uint8_t] channel
 * @param [uint16_t] on
 * @param [uint16_t] off
 * @return [*]
 */
uint8_t fml_servo_set_pwm(uint8_t channel, uint16_t on, uint16_t off)
{
    if (!s_init)
        return 1;
    if (channel >= PCA9685_CHANNEL_NUM)
        return 1;
    if (on > 4095 || off > 4095)
        return 1;

    uint8_t reg = PCA9685_LED0_ON_L + 4 * channel;
    uint8_t buf[4];
    buf[0] = on & 0xFF;
    buf[1] = (on >> 8) & 0xFF;
    buf[2] = off & 0xFF;
    buf[3] = (off >> 8) & 0xFF;

    return s_write_string(reg, buf, 4);
}

/**
 * @description: Servo angle set
 * @param [uint8_t] ch
 * @param [uint16_t] angle
 * @return [*]1:fail 0:successful
 */
uint8_t fml_servo_set_angle(uint8_t ch, uint16_t angle)
{
    if (!s_init)
        return 1;
    if (ch >= PCA9685_CHANNEL_NUM)
        return 1;
    if (angle > 180)
        angle = 180;

    uint16_t off = SERVO_PULSE_OFF_0DEG +
                   (uint32_t)(SERVO_PULSE_OFF_180DEG - SERVO_PULSE_OFF_0DEG) * angle / 180U;

    off += s_midoffset;

    /* Safety Limiting*/
    if (off < 80)
        off = 80;
    if (off > 550)
        off = 550;

    return fml_servo_set_pwm(ch, 0, off);
}

/**
 * @description: Servo set raw position value (PWM off count)
 * @param [uint8_t] ch channel
 * @param [uint16_t] pos raw pwm count value(80~550), will add midoffset auto
 * @return 1:fail 0:success
 */
uint8_t fml_servo_set_pos(uint8_t ch, uint16_t pos)
{
    if (!s_init)
        return 1;
    if (ch >= PCA9685_CHANNEL_NUM)
        return 1;

    int16_t raw = (int16_t)pos + s_midoffset;
    uint16_t off = (uint16_t)raw;

    /* Safety Limiting 和 fml_servo_set_angle 保持完全一致 */
    if (off < 80)
        off = 80;
    if (off > 550)
        off = 550;

    return fml_servo_set_pwm(ch, 0, off);
}

/**
 * @description: Servo release
 * @param [uint8_t] ch
 * @return [*]1:fail 0:successful
 */
uint8_t fml_servo_release(uint8_t ch)
{
    if (!s_init)
        return 1;
    if (ch >= PCA9685_CHANNEL_NUM)
        return 1;

    return fml_servo_set_pwm(ch, 0, 0);
}

/**
 * @description: Serovo midoffset set
 * @param [int16_t] offset
 * @return [*]middile offset
 */
void fml_servo_set_midoffset(int16_t offset)
{
    s_midoffset = offset;
}

/**
 * @description: Servo midoffset get
 * @return [*]middile offset
 */
int16_t fml_servo_get_midoffset(void)
{
    return s_midoffset;
}
