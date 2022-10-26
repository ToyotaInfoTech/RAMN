/**
  ******************************************************************************
  * @file    EEPROM_Emul/Porting/STM32L5/flash_interface.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the EEPROM
  *          emulation flash interface.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_INTERFACE_H
#define __FLASH_INTERFACE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l5xx_hal.h"
#include "stm32l5xx_ll_crc.h"
#include "stm32l5xx_ll_bus.h"

/** @addtogroup EEPROM_Emulation
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* Private types -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @addtogroup EEPROM_Private_Constants
  * @{
  */

/** @addtogroup Private_Other_Constants
  * @{
  */

#define BANK_SIZE               FLASH_BANK_SIZE         /*!< Alias to FLASH_BANK_SIZE definition from HAL STM32L4 */
#define EE_ACCESS_32BITS    /*!< Enable EEPROM 32bits R/W functions, only valid for flash allowing 64bits access*/

/* Page state header */
#define EE_PAGESTAT_ERASED      (uint64_t)0xFFFFFFFFFFFFFFFFU  /*!< State saved in 1st,2nd,3rd,4th data type of page header */
#define EE_PAGESTAT_RECEIVE     (uint64_t)0xAAAAAAAAAAAAAAAAU  /*!< State saved in 1st data type of page header */
#define EE_PAGESTAT_ACTIVE      (uint64_t)0xAAAAAAAAAAAAAAAAU  /*!< State saved in 2nd data type of page header */
#define EE_PAGESTAT_VALID       (uint64_t)0xAAAAAAAAAAAAAAAAU  /*!< State saved in 3rd data type of page header */
#define EE_PAGESTAT_ERASING     (uint64_t)0xAAAAAAAAAAAAAAAAU  /*!< State saved in 4th data type of page header */

/* Description of the 8 Bytes (64 bits) element in flash   */
/* Bit:  63                  32  31      16  15         0  */
/*       <--- Data Value ----->  <-unused->  <-VirtAddr->  */
#define EE_ELEMENT_SIZE         8U                            /*!< Size of element in Bytes */
#define EE_ELEMENT_TYPE         uint64_t                      /*!< Type of element */
#define EE_VIRTUALADDRESS_TYPE  uint16_t                      /*!< Type of Virtual Address */
#define EE_VIRTUALADDRESS_SHIFT 0U                            /*!< Bits Shifting to get Virtual Address in element */
#define EE_CRC_TYPE             uint16_t                      /*!< Type of Crc */
#define EE_CRC_SHIFT            16U                           /*!< Bits Shifting to get Crc in element */
#define EE_DATA_TYPE            uint32_t                      /*!< Type of Data */
#define EE_DATA_SHIFT           32U                           /*!< Bits Shifting to get Data value in element */
#define EE_MASK_VIRTUALADDRESS  (uint64_t)0x000000000000FFFFU
#define EE_MASK_CRC             (uint64_t)0x00000000FFFF0000U
#define EE_MASK_DATA            (uint64_t)0xFFFFFFFF00000000U
#define EE_MASK_FULL            (uint64_t)0xFFFFFFFFFFFFFFFFU

#define SECURE_FEATURE										  /*!< Informs the driver about the security features of the flash interface */
/**
  * @}
  */

/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/** @addtogroup EEPROM_Private_Macros
  * @{
  */

/**
  * @}
  */

/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/
/** @addtogroup EEPROM_Private_Functions
  * @{
  */
HAL_StatusTypeDef FI_WriteDoubleWord(uint32_t Address, uint64_t Data);
EE_Status FI_PageErase(uint32_t Page, uint16_t NbPages);
EE_Status FI_PageErase_IT(uint32_t Page, uint16_t NbPages);
EE_Status FI_DeleteCorruptedFlashAddress(uint32_t Address);
EE_Status FI_CheckBankConfig(void);
void FI_CacheFlush(void);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

#endif /* __FLASH_INTERFACE_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
