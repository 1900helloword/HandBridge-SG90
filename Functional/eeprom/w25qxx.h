/*
 * @Author: BigMAN
 * @Date: 2026-08-29 09:54:35
 * @LastEditors: BigMAN
 * @LastEditTime: 2026-08-29 15:05:28
 * @Contact: 13067060853@163.com
 * @Description: 该闪存驱动仅适用裸机+W25QXX / GD25QXX Flash
 * SPDX-License-Identifier: MIT
 */

#ifndef __W25Qxx_H
#define __W25Qxx_H

#include "main.h"
#include "gpio.h"
#include "spi.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SPI_FLASH_CS_LOW() HAL_GPIO_WritePin(GD25Q64_CS_GPIO_Port, GD25Q64_CS_Pin, GPIO_PIN_RESET);
#define SPI_FLASH_CS_HIGH() HAL_GPIO_WritePin(GD25Q64_CS_GPIO_Port, GD25Q64_CS_Pin, GPIO_PIN_SET);

    /**
     * @description: Eeprom init
     * @param [SPI_HandleTypeDef] *spi
     * @return [*]1:fail 0:successful
     */
    uint8_t fml_eeprom_init(SPI_HandleTypeDef *spi);

    /**
     * @description: Eeprom read buffer
     * @param [uint8_t] *pdata
     * @param [uint32_t] addr
     * @param [uint16_t] size
     * @return [*] none
     */
    void fml_eeprom_read_buf(uint8_t *pdata, uint32_t addr, uint16_t size);

    /**
     * @description: Eeprom write page
     * @param [uint8_t] *pdata
     * @param [uint32_t] addr
     * @param [uint16_t] size
     * @return [*] none
     */
    void fml_eeprom_write_page(uint8_t *pdata, uint32_t addr, uint16_t size);

    /**
     * @description: Eeprom write buffer
     * @param [uint8_t] *pdata
     * @param [uint32_t] addr
     * @param [uint32_t] size
     * @return [*] none
     */
    void fml_eeprom_write_buf(uint8_t *pdata, uint32_t addr, uint32_t size);

    /**
     * @description: Eeprom erase sector
     * @param [uint32_t] sector_addr
     * @return [*] none
     */
    void fml_eeprom_erase_sector(uint32_t sector_addr);

    /**
     * @description: Eeprom erase block
     * @return [*] none
     */
    void fml_eeprom_erase_block(void);

#ifdef __cplusplus
}
#endif

#endif /* __W25Qxx_H*/
