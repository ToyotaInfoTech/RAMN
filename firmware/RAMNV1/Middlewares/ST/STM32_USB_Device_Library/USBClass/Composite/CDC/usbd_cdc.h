/*
 * usbd_cdc.h
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2025 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
 *
 * This software component is licensed by TOYOTA MOTOR CORPORATION under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#ifndef ST_STM32_USB_DEVICE_LIBRARY_USBCLASS_CDC_USBD_CDC_H_
#define ST_STM32_USB_DEVICE_LIBRARY_USBCLASS_CDC_USBD_CDC_H_

USBD_StatusTypeDef USBD_CDC_Init(USBD_HandleTypeDef *pdev);
USBD_StatusTypeDef USBD_CDC_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req, uint8_t index);

#endif /* ST_STM32_USB_DEVICE_LIBRARY_USBCLASS_CDC_USBD_CDC_H_ */
