/*
 * @Author: BigMAN
 * @Date: 2026-09-04 08:47:14
 * @LastEditors: BigMAN
 * @LastEditTime: 2026-09-18 09:38:46
 * @Contact: 13067060853@163.com
 * @Description: 创建伪寄存器
 * SPDX-License-Identifier: MIT
 */
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>

#include "FTregister.h"

/* Variables ------------------------------------------------------------------*/
#define FT_SERVOS 10

#define FT_EEPROM_COUNT 20
#define FT_SRAM_COUNT 18

#define FT_SECTOR1 0
#define FT_SECTOR2 (FT_EEPROM_COUNT * FT_SERVOS)
#define FT_SECTOR3 (FT_EEPROM_COUNT + FT_SRAM_COUNT) * FT_SERVOS

static uint8_t s_register_buf[512] = {0};
static uint8_t s_register_lock = 0;

/* Static function ------------------------------------------------------------------*/
static uint8_t s_lock_check(void)
{
    if (s_register_lock)
        return 1;
    else
        return 0;
}
static void s_lock(void)
{
    s_register_lock = 1;
}

static void s_unlock(void)
{
    s_register_lock = 0;
}

static void s_write_buffer(uint8_t *buf, uint32_t addr, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        s_register_buf[addr + i] = buf[i];
    }
}

static void s_read_buffer(uint8_t *buf, uint32_t addr, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        buf[i] = s_register_buf[addr + i];
    }
}

static uint32_t s_fd_addr(uint8_t id, uint8_t addr)
{
    uint32_t index = 0;

    if (addr <= MAX_TORQUE_REG_H)
    {
        index = FT_SECTOR1 + (addr + (id - 1) * FT_EEPROM_COUNT);
    }

    if (addr == P_GAIN_REG || addr == D_GAIN_REG)
    {
        index = FT_SECTOR1 + ((addr - 3) + (id - 1) * FT_EEPROM_COUNT);
    }

    if (addr >= SRAM_START_VOID0 && addr <= GOAL_SPEED_REG_H)
    {
        index = FT_SECTOR2 + ((addr - SRAM_START_VOID0) + (id - 1) * FT_SRAM_COUNT);
    }

    if (addr >= CURRENT_POS_REG_L && addr <= ERROR_STATUS_REG)
    {
        index = FT_SECTOR2 + ((addr - CURRENT_POS_REG_L + 7) + (id - 1) * FT_SRAM_COUNT);
    }

    return index;
}

/* Public function ------------------------------------------------------------------*/
/**
 * @description: Register init
 * @return [*] none
 */
void bll_register_init(void)
{
    uint8_t eeprom_default[FT_EEPROM_COUNT] = {0, 5, 1, 5, 4, 0, 0, 0x5a, 1, 0, 0, (1023U >> 8U) & 0xFFU, 1023U & 0xffU, 70, 85, 55, (1000U >> 8U) & 0xFFU, 1000U & 0xffU, 32, 16};
    uint8_t sram_default[FT_SRAM_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 22, 0, 0, 0};

    for (uint8_t id = 1; id <= FT_SERVOS; id++)
    {
        s_write_buffer(eeprom_default, FT_SECTOR1 + ((id - 1) * FT_EEPROM_COUNT), FT_EEPROM_COUNT);
        bll_write_register(id, ID_REG, &id, 1);

        s_write_buffer(sram_default, FT_SECTOR2 + ((id - 1) * FT_SRAM_COUNT), FT_SRAM_COUNT);
    }
}

/**
 * @description: Register read
 * @param [uint8_t] id
 * @param [uint8_t] addr
 * @param [uint8_t] *out_buf
 * @param [uint8_t] len
 * @return [*] real len
 */
uint8_t bll_read_register(uint8_t id, uint8_t addr, uint8_t *out_buf, uint8_t len)
{
    if (s_lock_check() == 1)
        return 0;
    s_lock();

    if (id < 1 || id > FT_SERVOS)
        return 0;

    uint8_t real_len = 0;
    if (addr <= D_GAIN_REG)
    {
        if (addr + len > D_GAIN_REG)
            real_len = D_GAIN_REG - addr + 1;
        else
            real_len = len;
    }
    else if (addr >= SRAM_START_VOID0)
    {
        real_len = len;
    }
    else
    {
        s_unlock();
        return 0;
    }

    for (uint8_t i = 0; i < real_len; i++)
    {
        if ((addr > MAX_TORQUE_REG_H && addr < P_GAIN_REG) ||
            (addr > GOAL_SPEED_REG_H && addr < CURRENT_POS_REG_L) ||
            addr > ERROR_STATUS_REG)

            out_buf[i] = 0;
        else
            s_read_buffer(&out_buf[i], s_fd_addr(id, addr), 1);

        addr++;
    }

    s_unlock();
    return real_len;
}

/**
 * @description: Register write
 * @param [uint8_t] id
 * @param [uint8_t] addr
 * @param [uint8_t] *data
 * @param [uint8_t] len
 * @return [*] real len
 */
uint8_t bll_write_register(uint8_t id, uint8_t addr, uint8_t *data, uint8_t len)
{
    if (s_lock_check() == 1)
        return 0;
    s_lock();

    if (id < 1 || id > FT_SERVOS)
        return 0;

    uint8_t real_len = 0;
    if (addr <= D_GAIN_REG)
    {
        if (addr + len > D_GAIN_REG)
            real_len = D_GAIN_REG - addr + 1;
        else
            real_len = len;
    }
    else if (addr >= SRAM_START_VOID0)
    {
        real_len = len;
    }
    else
    {
        s_unlock();
        return 0;
    }

    for (uint8_t i = 0; i < real_len; i++)
    {
        if ((addr > MAX_TORQUE_REG_H && addr < P_GAIN_REG) ||
            (addr > GOAL_SPEED_REG_H && addr < CURRENT_POS_REG_L) ||
            addr > ERROR_STATUS_REG)

            __nop();
        else
            s_write_buffer(&data[i], s_fd_addr(id, addr), 1);

        addr++;
    }

    s_unlock();
    return real_len;
}
