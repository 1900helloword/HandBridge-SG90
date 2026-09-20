/*
 * @Author: BigMAN
 * @Date: 2026-08-29 09:54:35
 * @LastEditors: BigMAN
 * @LastEditTime: 2026-08-29 15:17:21
 * @Contact: 13067060853@163.com
 * @Description: 该闪存驱动仅适用裸机+W25QXX / GD25QXX Flash
 * SPDX-License-Identifier: MIT
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>

#include "w25qxx.h"

/* Variables ------------------------------------------------------------------*/
#define W25QXX_WRITE_ENABLE 0x06
#define W25QXX_WRITE_DISENABLE 0x04
#define W25QXX_READ_STATUS 0x05
#define W25QXX_WRITE_STATUS 0x01
#define W25QXX_READ_DATA 0x03
#define W25QXX_FASTREAD_DATA 0x0B
#define W25QXX_FASTREAD_DUAL 0x3B
#define W25QXX_PAGE_PROGRAM 0x02
#define W25QXX_BLOCK_ERASE 0xD8
#define W25QXX_SECTOR_ERASE 0x20
#define W25QXX_CHIP_ERASE 0xC7
#define W25QXX_DEVICE_ID 0x9F

SPI_HandleTypeDef *s_spi = NULL;

/* Static function ------------------------------------------------------------------*/
static uint8_t s_read_byte(void)
{
    uint8_t t_data, r_data;

    if (HAL_SPI_TransmitReceive(s_spi, &t_data, &r_data, 1, 0xFFFFFF) != HAL_OK)
    {
        r_data = 0xff;
    }
    return r_data;
}

static uint8_t s_write_byte(uint8_t byte)
{
    uint8_t r_data;

    if (HAL_SPI_TransmitReceive(s_spi, &byte, &r_data, 1, 0xFFFFFF) != HAL_OK)
    {
        return 1;
    }
    return 0;
}

static void s_write_enable(void)
{
    SPI_FLASH_CS_LOW();
    s_write_byte(W25QXX_WRITE_ENABLE);
    SPI_FLASH_CS_HIGH();
}

static void s_write_wait(void)
{
    uint8_t state = 0;

    SPI_FLASH_CS_LOW();

    s_write_byte(W25QXX_READ_STATUS);

    do
    {
        state = s_read_byte();
    } while ((state & 0x01) == SET);

    SPI_FLASH_CS_HIGH();
}

/* Public function ------------------------------------------------------------------*/
/**
 * @description: Eeprom init
 * @param [SPI_HandleTypeDef] *spi
 * @return [*]1:fail 0:successful
 */
uint8_t fml_eeprom_init(SPI_HandleTypeDef *spi)
{
    s_spi = spi;
    uint32_t temp, temp0, temp1, temp2;

    SPI_FLASH_CS_LOW();
    s_write_byte(W25QXX_DEVICE_ID);

    temp0 = s_read_byte();
    temp1 = s_read_byte();
    temp2 = s_read_byte();

    SPI_FLASH_CS_HIGH();

    temp = (temp0 << 16) | (temp1 << 8) | temp2;

    if (temp > 0 && temp < 0xFFFFFFFF)
        return 0;
    else
        return 1;
}

/**
 * @description: Eeprom read buffer
 * @param [uint8_t] *pdata
 * @param [uint32_t] addr
 * @param [uint16_t] size
 * @return [*] none
 */
void fml_eeprom_read_buf(uint8_t *pdata, uint32_t addr, uint16_t size)
{
    SPI_FLASH_CS_LOW();

    s_write_byte(W25QXX_READ_DATA);

    s_write_byte((addr & 0xFF0000) >> 16);
    s_write_byte((addr & 0xFF00) >> 8);
    s_write_byte(addr & 0xFF);

    while (size--)
    {
        *pdata = s_read_byte();
        pdata++;
    }

    SPI_FLASH_CS_HIGH();
}

/**
 * @description: Eeprom write page
 * @param [uint8_t] *pdata
 * @param [uint32_t] addr
 * @param [uint16_t] size
 * @return [*] none
 */
void fml_eeprom_write_page(uint8_t *pdata, uint32_t addr, uint16_t size)
{
    uint16_t i;

    s_write_enable();

    SPI_FLASH_CS_LOW();

    s_write_byte(W25QXX_PAGE_PROGRAM);
    s_write_byte((uint8_t)((addr) >> 16));
    s_write_byte((uint8_t)((addr) >> 8));
    s_write_byte((uint8_t)addr);

    for (i = 0; i < size; i++)
    {
        s_write_byte(pdata[i]);
    }

    SPI_FLASH_CS_HIGH();
    s_write_wait();
}

/**
 * @description: Eeprom write buffer
 * @param [uint8_t] *pdata
 * @param [uint32_t] addr
 * @param [uint32_t] size
 * @return [*] none
 */
void fml_eeprom_write_buf(uint8_t *pdata, uint32_t addr, uint32_t size)
{
    uint32_t page_remain;

    page_remain = 256 - addr % 256;

    if (size <= page_remain)
    {
        page_remain = size;
    }
    while (1)
    {
        fml_eeprom_write_page(pdata, addr, page_remain);

        if (size == page_remain)
            break;
        else
        {
            pdata += page_remain;
            addr += page_remain;

            size -= page_remain;
            if (size > 256)
                page_remain = 256;
            else
                page_remain = size;
        }
    }
}

/**
 * @description: Eeprom erase sector
 * @param [uint32_t] sector_addr
 * @return [*] none
 */
void fml_eeprom_erase_sector(uint32_t sector_addr)
{
    s_write_enable();
    s_write_wait();

    SPI_FLASH_CS_LOW();
    s_write_byte(W25QXX_SECTOR_ERASE);
    s_write_byte((sector_addr & 0xFF0000) >> 16);
    s_write_byte((sector_addr & 0xFF00) >> 8);
    s_write_byte(sector_addr & 0xFF);

    SPI_FLASH_CS_HIGH();

    s_write_wait();
}

/**
 * @description: Eeprom erase block
 * @return [*] none
 */
void fml_eeprom_erase_block(void)
{
    s_write_enable();

    SPI_FLASH_CS_LOW();
    s_write_byte(W25QXX_CHIP_ERASE);
    SPI_FLASH_CS_HIGH();

    s_write_wait();
}
