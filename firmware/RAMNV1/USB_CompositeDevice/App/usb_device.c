/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : usb_device.c
 * @version        : v3.0_Cube
 * @brief          : This file implements the USB Device
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 * <h2><center>&copy; Copyright (c) 2025 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include <usbd_gsusb_if.h>
#include "../../USB_CompositeDevice/App/usb_device.h"

#include "main.h"

#include "usbd_composite.h"
#include "usbd_core.h"
#include "usbd_cdc_if.h"
#include "usbd_desc.h"

/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

extern void Error_Handler(void);
/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceFS;
extern USBD_DescriptorsTypeDef MDC_Desc;
/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */
void MX_USB_Device_DeInit(void)
{
	if (USBD_Stop(&hUsbDeviceFS) != USBD_OK) {
		Error_Handler();
	}

	if (USBD_DeInit(&hUsbDeviceFS) != USBD_OK) {
		Error_Handler();
	}


}
/* USER CODE END 1 */

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_Device_Init(void)
{
  /* USER CODE BEGIN USB_Device_Init_PreTreatment */
#if defined(ENABLE_USB)
  /* USER CODE END USB_Device_Init_PreTreatment */

  /* Init Device Library, add supported class and start the library. */
  if (USBD_Init(&hUsbDeviceFS, &MDC_Desc, DEVICE_FS) != USBD_OK) {
    Error_Handler();
  }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_Composite) != USBD_OK) {
    Error_Handler();
  }
#ifdef ENABLE_CDC
  // Set CDC Interface
  if (USBD_Composite_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS, 0) != USBD_OK) {
    Error_Handler();
  }
#endif
#ifdef ENABLE_GSUSB
  // Set SocketCAN Interface
  if (USBD_Composite_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_GSUSB, 1) != USBD_OK) {
    Error_Handler();
  }
#endif
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Device_Init_PostTreatment */
#endif
  /* USER CODE END USB_Device_Init_PostTreatment */
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
