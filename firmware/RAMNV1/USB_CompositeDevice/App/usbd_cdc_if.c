/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : usbd_cdc_if.c
 * @version        : v3.0_Cube
 * @brief          : Usb device for Virtual Com Port.
 ******************************************************************************
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

/* Includes ------------------------------------------------------------------*/
#include "ramn_config.h"
#ifdef ENABLE_CDC
#include "../../USB_CompositeDevice/App/usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include "ramn_usb.h"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */
extern RAMN_USB_Status_t RAMN_USB_Config;
/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
 * @brief Usb device library.
 * @{
 */

/** @addtogroup USBD_CDC_IF
 * @{
 */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
 * @brief Private types.
 * @{
 */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
 * @brief Private defines.
 * @{
 */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
 * @brief Private macros.
 * @{
 */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
 * @brief Private variables.
 * @{
 */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
__attribute__ ((section (".buffers"))) uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
__attribute__ ((section (".buffers"))) uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
static StreamBufferHandle_t* USBD_recvBuffer = NULL;
static osThreadId_t* USBD_recvTask = NULL;
static osThreadId_t* USBD_sendTask = NULL;
static uint16_t currentIndex = 0;
__attribute__ ((section (".buffers"))) static uint8_t  recvBuf[USB_COMMAND_BUFFER_SIZE];
/* USER CODE END PRIVATE_VARIABLES */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
 * @brief Public variables.
 * @{
 */

extern USBD_HandleTypeDef hUsbDeviceFS;
static USBD_CDC_LineCodingTypeDef lineCoding;
/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
 * @brief Private functions declaration.
 * @{
 */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
void RAMN_CDC_Init(StreamBufferHandle_t* pBuffer, osThreadId_t* pRecvTask, osThreadId_t* pSendTask)
{
	USBD_recvBuffer = pBuffer;
	USBD_recvTask = pRecvTask;
	USBD_sendTask = pSendTask;
}

uint8_t RAMN_CDC_GetTXStatus()
{
	USBD_Composite_HandleTypeDef *hcdc = (USBD_Composite_HandleTypeDef*)hUsbDeviceFS.pClassData;
	if (hcdc == NULL || hcdc->TxState[0] != 0)
	{
		return USBD_BUSY;
	}
	else
	{
		return USBD_OK;
	}
}
/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
 * @}
 */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
		CDC_Init_FS,
		CDC_DeInit_FS,
		CDC_Control_FS,
		CDC_Receive_FS,
		CDC_TransmitCplt_FS
};

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Initializes the CDC media low layer over the FS USB IP
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Init_FS(void)
{
	/* USER CODE BEGIN 3 */
	/* Set Application Buffers */
	USBD_Composite_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, APP_TX_DATA_SIZE, 0);
	USBD_Composite_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS, 0);
	return (USBD_OK);
	/* USER CODE END 3 */
}

/**
 * @brief  DeInitializes the CDC media low layer
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_DeInit_FS(void)
{
	/* USER CODE BEGIN 4 */
	return (USBD_OK);
	/* USER CODE END 4 */
}

/**
 * @brief  Manage the CDC class requests
 * @param  cmd: Command code
 * @param  pbuf: Buffer containing command data (request parameters)
 * @param  length: Number of data to be sent (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
	/* USER CODE BEGIN 5 */
	switch(cmd)
	{
	case CDC_SEND_ENCAPSULATED_COMMAND:

		break;

	case CDC_GET_ENCAPSULATED_RESPONSE:

		break;

	case CDC_SET_COMM_FEATURE:

		break;

	case CDC_GET_COMM_FEATURE:

		break;

	case CDC_CLEAR_COMM_FEATURE:

		break;

		/*******************************************************************************/
		/* Line Coding Structure                                                       */
		/*-----------------------------------------------------------------------------*/
		/* Offset | Field       | Size | Value  | Description                          */
		/* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
		/* 4      | bCharFormat |   1  | Number | Stop bits                            */
		/*                                        0 - 1 Stop bit                       */
		/*                                        1 - 1.5 Stop bits                    */
		/*                                        2 - 2 Stop bits                      */
		/* 5      | bParityType |  1   | Number | Parity                               */
		/*                                        0 - None                             */
		/*                                        1 - Odd                              */
		/*                                        2 - Even                             */
		/*                                        3 - Mark                             */
		/*                                        4 - Space                            */
		/* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
		/*******************************************************************************/
	case CDC_SET_LINE_CODING:
		lineCoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) | (pbuf[2] << 16) | (pbuf[3] << 24));
		lineCoding.format     = pbuf[4];
		lineCoding.paritytype = pbuf[5];
		lineCoding.datatype   = pbuf[6];
		break;

	case CDC_GET_LINE_CODING:

		pbuf[0] = (uint8_t)(lineCoding.bitrate);
		pbuf[1] = (uint8_t)(lineCoding.bitrate >> 8);
		pbuf[2] = (uint8_t)(lineCoding.bitrate >> 16);
		pbuf[3] = (uint8_t)(lineCoding.bitrate >> 24);
		pbuf[4] = lineCoding.format;
		pbuf[5] = lineCoding.paritytype;
		pbuf[6] = lineCoding.datatype;
		break;

	case CDC_SET_CONTROL_LINE_STATE:
		if ((((USBD_SetupReqTypedef *)pbuf)->wValue & 0x0001) != 0U)
		{
			//Port has been opened
			if (USBD_serialOpenCallback_ptr != NULL) (*USBD_serialOpenCallback_ptr)(&hUsbDeviceFS, 0);
		}
		else
		{
			//Port has been close
			if (USBD_serialCloseCallback_ptr != NULL) (*USBD_serialCloseCallback_ptr)(&hUsbDeviceFS, 0);
		}
		break;

	case CDC_SEND_BREAK:

		break;

	default:
		break;
	}

	return (USBD_OK);
	/* USER CODE END 5 */
}

/**
 * @brief  Data received over USB OUT endpoint are sent over CDC interface
 *         through this function.
 *
 *         @note
 *         This function will issue a NAK packet on any OUT packet received on
 *         USB endpoint until exiting this function. If you exit this function
 *         before transfer is complete on CDC interface (ie. using DMA controller)
 *         it will result in receiving more data while previous ones are still
 *         not sent.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
	/* USER CODE BEGIN 6 */
	//USBD_Composite_SetRxBuffer(&hUsbDeviceFS, &Buf[0], 0);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	for(uint32_t i=0; i < *Len; i++)
	{
		if (currentIndex >= sizeof(recvBuf))
		{
			// No room in buffer, reset
			currentIndex = 0;
			if (USBD_errorCallback_ptr != NULL) (*USBD_errorCallback_ptr)(&hUsbDeviceFS);
		}
		else
		{
			if((Buf[i] == '\n') || (Buf[i] == 0))
			{
				// newline or zero character, we just ignore it.
			}
			else if(Buf[i] != '\r') // Regular character, we add it to the buffer
			{
				recvBuf[currentIndex] = Buf[i];
				currentIndex++;
			}
			else // We reached CR, must send command for processing
			{
				if ((currentIndex > 0) && (currentIndex <= USB_COMMAND_BUFFER_SIZE)) //Don't forward invalid commands
				{
					if(USBD_recvBuffer != NULL)
					{
						if (xStreamBufferSendFromISR(*USBD_recvBuffer, &currentIndex, 2U, NULL ) != 2U)
						{
							//If a callback function has been registered, report issue
							//if (USBD_errorCallback_ptr != NULL) (*USBD_errorCallback_ptr)(&hUsbDeviceFS);
							RAMN_USB_Config.USBErrCnt++;
						}
						else
						{
							if (xStreamBufferSendFromISR(*USBD_recvBuffer, recvBuf, currentIndex, NULL ) != currentIndex)
							{
								//if (USBD_errorCallback_ptr != NULL) (*USBD_errorCallback_ptr)(&hUsbDeviceFS);
								RAMN_USB_Config.USBErrCnt++;
							}
							else
							{
								vTaskNotifyGiveFromISR(*USBD_recvTask, &xHigherPriorityTaskWoken);
								portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
							}
						}
					}
				}
				currentIndex = 0;
			}
		}
	}
	USBD_Composite_ReceivePacket(&hUsbDeviceFS, CDC_OUT_EP);

	return (USBD_OK);
	/* USER CODE END 6 */
}

/**
 * @brief  CDC_Transmit_FS
 *         Data to send over USB IN endpoint are sent over CDC interface
 *         through this function.
 *         @note
 *
 *
 * @param  Buf: Buffer of data to be sent
 * @param  Len: Number of data to be sent (in bytes)
 * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
 */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
	uint8_t result = USBD_OK;
	/* USER CODE BEGIN 7 */
	if (RAMN_CDC_GetTXStatus() != USBD_OK){
		return USBD_BUSY;
	}

	USBD_Composite_SetTxBuffer(&hUsbDeviceFS, Buf, Len, 0);
	result = USBD_Composite_TransmitPacket(&hUsbDeviceFS, 0);

	/* USER CODE END 7 */
	return result;
}

/**
 * @brief  CDC_TransmitCplt_FS
 *         Data transmitted callback
 *
 *         @note
 *         This function is IN transfer complete callback used to inform user that
 *         the submitted Data is successfully sent over USB.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
	uint8_t result = USBD_OK;
	/* USER CODE BEGIN 13 */
	UNUSED(Buf);
	UNUSED(Len);
	UNUSED(epnum);
	if(USBD_sendTask != NULL)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		vTaskNotifyGiveFromISR(*USBD_sendTask, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
	/* USER CODE END 13 */
	return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
 * @}
 */

/**
 * @}
 */
#endif /* ENABLE_USB */

typedef int usbd_cdc_if_prevent_empty_translation_unit;

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
