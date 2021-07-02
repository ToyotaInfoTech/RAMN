/**
  ******************************************************************************
  * @file    EEPROM_Emul/Core/eeprom_emul_types.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the EEPROM
  *          emulation firmware library.
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
#ifndef __EEPROM_EMUL_TYPES_H
#define __EEPROM_EMUL_TYPES_H

/** @addtogroup EEPROM_Emulation
  * @{
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup EEPROM_Exported_Constants EEPROM Exported Constants
  * @{
  */

/** @defgroup Exported_Other_Constants Exported Other Constants
  * @{
  */

/**
  * @brief  EE Status enum definition.
  */
/* Define of the return value */
typedef enum {
  /* External return codes : ok */
  EE_OK = 0U,

  /* External return codes : errors */
  EE_ERASE_ERROR,
  EE_WRITE_ERROR,
  EE_ERROR_NOACTIVE_PAGE,
  EE_ERROR_NOERASE_PAGE,
  EE_ERROR_NOERASING_PAGE,
  EE_ERROR_NOACTIVE_NORECEIVE_NOVALID_PAGE,
  EE_NO_DATA,
  EE_INVALID_VIRTUALADDRESS,
  EE_INVALID_PAGE,
  EE_INVALID_PAGE_SEQUENCE,
  EE_INVALID_ELEMENT,
  EE_TRANSFER_ERROR,
  EE_DELETE_ERROR,
  EE_INVALID_BANK_CFG,

  /* Internal return code */
  EE_NO_PAGE_FOUND,
  EE_PAGE_NOTERASED,
  EE_PAGE_ERASED,
  EE_PAGE_FULL,

  /* External return code : action required */
  EE_CLEANUP_REQUIRED = 0x100U,
  
#ifdef DUALCORE_FLASH_SHARING
  /* Value returned when a program or erase operation is requested but
  * the flash is already used by CPU2 */
  EE_FLASH_USED,
  EE_SEM_TIMEOUT,
#endif

} EE_Status;

/* Type of page erasing:
       EE_FORCED_ERASE     --> pages to erase are erased unconditionnally
       EE_CONDITONAL_ERASE --> pages to erase are erased only if not fully erased */
typedef enum {
   EE_FORCED_ERASE,
   EE_CONDITIONAL_ERASE
} EE_Erase_type;

#ifdef DUALCORE_FLASH_SHARING
/* Type of write operations:
       EE_TRANSFER         --> Used by WriteDoubleWord to know when the operation ongoing is a transfer
       EE_SIMPLE_WRITE     --> Used by WriteDoubleWord to know when the operation ongoing is a simple writing */
typedef enum {
   EE_TRANSFER,
   EE_SIMPLE_WRITE,
   EE_SET_PAGE,
   EE_INIT_WRITE
} EE_Write_type;
#endif

/* Masks of EE_Status return codes */
#define EE_STATUSMASK_ERROR   (uint16_t)0x00FFU /*!< Mask on EE_Status return code, selecting error codes */
#define EE_STATUSMASK_CLEANUP (uint16_t)0x0100U /*!< Mask on EE_Status return code, selecting cleanup request codes */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __EEPROM_EMUL_TYPES_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
