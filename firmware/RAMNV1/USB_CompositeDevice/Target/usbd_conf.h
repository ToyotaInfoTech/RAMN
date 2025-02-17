/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_conf.h
  * @version        : v3.0_Cube
  * @brief          : Header for usbd_conf.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32l5xx.h"
#include "stm32l5xx_hal.h"
#include "ramn_config.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/** @addtogroup USBD_OTG_DRIVER
  * @brief Driver for Usb device.
  * @{
  */

/** @defgroup USBD_CONF USBD_CONF
  * @brief Configuration file for Usb otg low level driver.
  * @{
  */

/** @defgroup USBD_CONF_Exported_Variables USBD_CONF_Exported_Variables
  * @brief Public variables.
  * @{
  */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */
/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_Defines USBD_CONF_Exported_Defines
  * @brief Defines for configuration of the Usb device.
  * @{
  */

// Note there is currently an issue with CDC_WINDEX.
// Setting CDC_WINDEX to 0x04 when ENABLE_CDC and ENABLE_GSUSB are both active lead to a hardware fault with Linux when load gs_usb
// Setting CDC_WINDEX to any other value than 0x04 when only ENABLE_CDC is active leads to a delayed start on Windows (USBD_Composite_Setup is called with req->wIndex == 4U regardless of the value of CDC_WINDEX).
// TODO: fix

#if defined(ENABLE_CDC) && defined(ENABLE_GSUSB)
#define USBD_MAX_NUM_INTERFACES     	4U
#define USB_COMPOSITE_CONFIG_DESC_SIZ   116
#define GSUSB_WINDEX                    0x00   /* gs_usb interface number */
#define CDC_WINDEX                      0x02   /* CDC interface number */ // Read warning above
#endif

#if defined(ENABLE_CDC) && !defined(ENABLE_GSUSB)
#define USBD_MAX_NUM_INTERFACES			2U
#define USB_COMPOSITE_CONFIG_DESC_SIZ   75
#define CDC_WINDEX                      0x04   /* CDC interface number */ // Read warning above
#endif


#if !defined(ENABLE_CDC) && defined(ENABLE_GSUSB)
#define USBD_MAX_NUM_INTERFACES         2U
#define USB_COMPOSITE_CONFIG_DESC_SIZ   50
#define GSUSB_WINDEX                    0x00   /* gs_usb interface number */
#endif

#if defined(ENABLE_CDC) && defined(ENABLE_GSUSB) && (CDC_WINDEX == 0x04)
#error invalid configuration (see warning above)
#endif

#if defined(ENABLE_CDC) && !defined(ENABLE_GSUSB) && (CDC_WINDEX != 0x04)
#error invalid configuration (see warning above)
#endif

/*---------- -----------*/
#define USBD_MAX_STR_DESC_SIZ       512U
/*---------- -----------*/
#define USBD_DEBUG_LEVEL            0U
/*---------- -----------*/
#define USBD_LPM_ENABLED            0U
/*---------- -----------*/
#define USBD_SELF_POWERED           0U

/****************************************/
/* #define for FS and HS identification */
#define DEVICE_FS 		1

/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_Macros USBD_CONF_Exported_Macros
  * @brief Aliases.
  * @{
  */

/* Memory management macros */
/** Alias for memory allocation. */
#define USBD_malloc         (void *)USBD_static_malloc
#define USBD_malloc2        (void *)USBD_static_malloc2

/** Alias for memory release. */
#define USBD_free           USBD_static_free

/** Alias for memory set. */
#define USBD_memset         memset

/** Alias for memory copy. */
#define USBD_memcpy         memcpy

/** Alias for delay. */
#define USBD_Delay          HAL_Delay
/* DEBUG macros */

#if (USBD_DEBUG_LEVEL > 0)
#define USBD_UsrLog(...)    printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)

#define USBD_ErrLog(...)    printf("ERROR: ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_ErrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 2)
#define USBD_DbgLog(...)    printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_DbgLog(...)
#endif

/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_Types USBD_CONF_Exported_Types
  * @brief Types.
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_FunctionsPrototype USBD_CONF_Exported_FunctionsPrototype
  * @brief Declaration of public functions for Usb device.
  * @{
  */

/* Exported functions -------------------------------------------------------*/
void *USBD_static_malloc(uint32_t size);
void *USBD_static_malloc2(uint32_t size);
void USBD_static_free(void *p);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CONF__H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
