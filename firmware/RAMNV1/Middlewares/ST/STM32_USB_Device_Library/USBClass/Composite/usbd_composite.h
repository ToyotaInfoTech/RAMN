/*
 * usbd_composite.h
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_COMPOSITE_H
#define __USB_COMPOSITE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "usbd_def.h"
#include "usbd_ioreq.h"
#include "../../USBClass/Composite/gs_usb/usbd_gs_usb.h"

/*---------------------------------------------------------------------*/
/*  Endpoint definitions                                               */
/*---------------------------------------------------------------------*/
#define GSUSB_IN_EP                                    0x81U  /* EP1 for gs_usb data IN */
#define GSUSB_OUT_EP                                   0x02U  /* EP2 for gs_usb data OUT */

#define CDC_IN_EP                                   0x84U  /* EP4 for CDC data IN */
#define CDC_OUT_EP                                  0x04U  /* EP4 for CDC data OUT */
#define CDC_CMD_EP                                  0x83U  /* EP3 for CDC commands */

#define CAN_DATA_MAX_PACKET_SIZE   64  /* Endpoint IN & OUT Packet size */
#define CAN_CMD_PACKET_SIZE        64  /* Control Endpoint Packet size */

/*---------------------------------------------------------------------*/
/*  CDC definitions                                                    */
/*---------------------------------------------------------------------*/
#ifndef CDC_HS_BINTERVAL
#define CDC_HS_BINTERVAL                            0x10U
#endif /* CDC_HS_BINTERVAL */

#ifndef CDC_FS_BINTERVAL
#define CDC_FS_BINTERVAL                            0x10U
#endif /* CDC_FS_BINTERVAL */

/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define CDC_DATA_HS_MAX_PACKET_SIZE                 512U  /* Endpoint IN & OUT Packet size */
#define CDC_DATA_FS_MAX_PACKET_SIZE                 64U  /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SIZE                         8U  /* Control Endpoint Packet size */

#define CDC_DATA_FS_IN_PACKET_SIZE                  CDC_DATA_FS_MAX_PACKET_SIZE
#define CDC_DATA_FS_OUT_PACKET_SIZE                 CDC_DATA_FS_MAX_PACKET_SIZE

#define CDC_REQ_MAX_DATA_SIZE                       0x7U
#define CDC_SEND_ENCAPSULATED_COMMAND               0x00U
#define CDC_GET_ENCAPSULATED_RESPONSE               0x01U
#define CDC_SET_COMM_FEATURE                        0x02U
#define CDC_GET_COMM_FEATURE                        0x03U
#define CDC_CLEAR_COMM_FEATURE                      0x04U
#define CDC_SET_LINE_CODING                         0x20U
#define CDC_GET_LINE_CODING                         0x21U
#define CDC_SET_CONTROL_LINE_STATE                  0x22U
#define CDC_SEND_BREAK                              0x23U

/*---------------------------------------------------------------------*/
/*  gs_usb definitions                                                    */
/*---------------------------------------------------------------------*/
#define NUM_CAN_CHANNEL 1

typedef struct
{
  uint32_t  data[CDC_DATA_HS_MAX_PACKET_SIZE / 4U];      /* Force 32bits alignment */
  uint8_t   CmdOpCode;
  uint8_t   CmdLength;
  uint8_t  *RxBuffer[3];
  uint8_t  *TxBuffer[3];
  uint32_t  RxLength[3];
  uint32_t  TxLength[3];

  __IO uint32_t TxState[3];
  __IO uint32_t RxState[3];

  USBD_GS_CAN_HandleTypeDef *hcan;

} USBD_Composite_HandleTypeDef;

extern USBD_ClassTypeDef USBD_Composite;
#define USBD_CDC_CLASS &USBD_CDC

uint8_t USBD_Composite_RegisterInterface(
	USBD_HandleTypeDef  *pdev,
    void                *fops,
	uint32_t             idx
);
uint8_t USBD_Composite_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff,
                             uint32_t length, uint8_t idx);
uint8_t USBD_Composite_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint8_t idx);
uint8_t USBD_Composite_ReceivePacket(USBD_HandleTypeDef *pdev, uint32_t epnum);
uint8_t USBD_Composite_TransmitPacket(USBD_HandleTypeDef *pdev, uint8_t idx);


extern void (*USBD_errorCallback_ptr)(USBD_HandleTypeDef* hUsbDeviceFS);
extern void (*USBD_serialOpenCallback_ptr)(USBD_HandleTypeDef* hUsbDeviceFS, uint8_t index);
extern void (*USBD_serialCloseCallback_ptr)(USBD_HandleTypeDef* hUsbDeviceFS, uint8_t index);
#ifdef __cplusplus
}
#endif

#endif  /* __USB_COMPOSITE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
