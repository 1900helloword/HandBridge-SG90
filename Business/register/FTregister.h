/*
 * @Author: BigMAN
 * @Date: 2026-09-04 08:47:29
 * @LastEditors: BigMAN
 * @LastEditTime: 2026-09-11 16:56:44
 * @Contact: 13067060853@163.com
 * @Description: 创建伪寄存器
 * SPDX-License-Identifier: MIT
 */

#ifndef __FTREG_H
#define __FTREG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

/* EEPROM Register */
#define VERSION_MAJOR_REG 0x00U
#define VERSION_MINOR_REG 0x01U
#define STORAGE_MODE_REG 0x02U
#define SERVO_MAJOR_REG 0x03U
#define SERVO_MINOR_REG 0x04U
#define ID_REG 0x05U
#define BAUDRATE_REG 0x06U
#define RESERVED07_REG 0x07U
#define RESPOND_LEVEL_REG 0x08U
#define MIN_ANGLE_REG_L 0x09U
#define MIN_ANGLE_REG_H 0x0AU
#define MAX_ANGLE_REG_L 0x0BU
#define MAX_ANGLE_REG_H 0x0CU
#define MAX_TEMP_REG 0x0DU
#define MAX_VOLTAGE_REG 0x0EU
#define MIN_VOLTAGE_REG 0x0FU
#define MAX_TORQUE_REG_L 0x10U
#define MAX_TORQUE_REG_H 0x11U

#define P_GAIN_REG 0x15U
#define D_GAIN_REG 0x16U

/* SRAM Register */
#define SRAM_START_VOID0 0x29U
#define GOAL_POS_REG_L 0x2AU
#define GOAL_POS_REG_H 0x2BU
#define GOAL_TIME_REG_L 0x2CU
#define GOAL_TIME_REG_H 0x2DU
#define GOAL_SPEED_REG_L 0x2EU
#define GOAL_SPEED_REG_H 0x2FU

#define CURRENT_POS_REG_L 0x38U
#define CURRENT_POS_REG_H 0x39U
#define CURRENT_SPEED_REG_L 0x3AU
#define CURRENT_SPEED_REG_H 0x3BU
#define LOAD_REG_L 0x3CU
#define LOAD_REG_H 0x3DU
#define VOLTAGE_REG 0x3EU
#define TEMPERATURE_REG 0x3FU
#define REGISTERED_INST_REG 0x40U
#define MOVING_STATUS_REG 0x41U
#define ERROR_STATUS_REG 0x42U

    /**
     * @description: Register init
     * @return [*] none
     */
    void bll_register_init(void);

    /**
     * @description: Register read
     * @param [uint8_t] id
     * @param [uint8_t] addr
     * @param [uint8_t] *out_buf
     * @param [uint8_t] len
     * @return [*] real len
     */
    uint8_t bll_read_register(uint8_t id, uint8_t addr, uint8_t *out_buf, uint8_t len);

    /**
     * @description: Register write
     * @param [uint8_t] id
     * @param [uint8_t] addr
     * @param [uint8_t] *data
     * @param [uint8_t] len
     * @return [*] real len
     */
    uint8_t bll_write_register(uint8_t id, uint8_t addr, uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* __FTREG_H */
