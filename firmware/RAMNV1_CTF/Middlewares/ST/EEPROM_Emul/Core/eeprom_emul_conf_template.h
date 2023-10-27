/**
  ******************************************************************************
  * @file    eeprom_emul_conf.h
  * @author  MCD Application Team
  * @brief   EEPROM emulation configuration file.
  *          This file should be copied to the application folder and renamed
  *          to eeprom_emul_conf.h.
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

/** @addtogroup EEPROM_Emulation
  * @{
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_EMUL_CONF_H
#define __EEPROM_EMUL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Private constants ---------------------------------------------------------*/
/** @addtogroup EEPROM_Private_Constants
  * @{
  */

/** @defgroup Private_Configuration_Constants Private Configuration Constants
  * @{
  */

/* Configuration of eeprom emulation in flash, can be custom */
#define START_PAGE_ADDRESS      0x08080000U /*!< Start address of the 1st page in flash, for EEPROM emulation */
#define CYCLES_NUMBER           1U   /*!< Number of 10Kcycles requested, minimum 1 for 10Kcycles (default),
                                        for instance 10 to reach 100Kcycles. This factor will increase
                                        pages number */
#define GUARD_PAGES_NUMBER      2U   /*!< Number of guard pages avoiding frequent transfers (must be multiple of 2): 0,2,4.. */

/* Configuration of crc calculation for eeprom emulation in flash */
#define CRC_POLYNOMIAL_LENGTH   LL_CRC_POLYLENGTH_16B /* CRC polynomial lenght 16 bits */
#define CRC_POLYNOMIAL_VALUE    0x8005U /* Polynomial to use for CRC calculation */

/**
  * @}
  */

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/** @defgroup EEPROM_Exported_Constants EEPROM Exported Constants
  * @{
  */

/** @defgroup Exported_Configuration_Constants Exported Configuration Constants
  * @{
  */
#define NB_OF_VARIABLES         1000U  /*!< Number of variables to handle in eeprom */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

#endif /* __EEPROM_EMUL_CONF_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
