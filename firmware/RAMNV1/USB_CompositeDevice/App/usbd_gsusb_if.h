/* USER CODE BEGIN Header */
/**
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  * <h2><center>&copy; Copyright (c) 2025 TOYOTA MOTOR CORPORATION.
  * ALL RIGHTS RESERVED.</center></h2>
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
#ifndef __USBD_GSUSB_IF_H__
#define __USBD_GSUSB_IF_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_composite.h"

/* USER CODE BEGIN INCLUDE */
#include "cmsis_os.h"
#include "stream_buffer.h"
/* USER CODE END INCLUDE */

/* USER CODE BEGIN EXPORTED_DEFINES */
/* Define size for the receive and transmit buffer over SocketCAN */
/* It's up to user to redefine and/or remove those define */
#define GSUSB_RX_DATA_SIZE  			1024
#define GSUSB_TX_DATA_SIZE  			1024

 typedef struct _USBD_GSUSB_Itf
 {
   int8_t (* Init)(USBD_HandleTypeDef *pdev, osMessageQueueId_t q_frame_pool, osMessageQueueId_t q_from_host);
   int8_t (* DeInit)(USBD_HandleTypeDef *pdev);
   int8_t (* Receive)(USBD_HandleTypeDef *pdev, uint8_t *Buf, uint32_t *Len);
   int8_t (* Transmit)(USBD_HandleTypeDef *pdev, uint8_t* Buf, uint16_t Len);
 } USBD_GSUSB_ItfTypeDef;


/** CDC Interface callback. */
extern USBD_GSUSB_ItfTypeDef USBD_Interface_fops_GSUSB;

/* USER CODE BEGIN EXPORTED_VARIABLES */
extern void (*USBD_errorCallback_ptr)(USBD_HandleTypeDef* hUsbDeviceFS);
extern void (*USBD_serialOpenCallback_ptr)(USBD_HandleTypeDef* hUsbDeviceFS, uint8_t index);
extern void (*USBD_serialCloseCallback_ptr)(USBD_HandleTypeDef* hUsbDeviceFS, uint8_t index);
/* USER CODE END EXPORTED_VARIABLES */

/* USER CODE BEGIN EXPORTED_FUNCTIONS */

// Initializes low level USB features
void RAMN_GSUSB_Init(StreamBufferHandle_t* pBuffer, osThreadId_t* pRecvTask, osThreadId_t* pSendTask);
int8_t GSUSB_Transmit(USBD_HandleTypeDef *pdev, uint8_t* Buf, uint16_t Len);

// Returns current transmit Status of USB module
uint8_t RAMN_GSUSB_GetTXStatus();
/* USER CODE END EXPORTED_FUNCTIONS */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_GSUSB_IF_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
