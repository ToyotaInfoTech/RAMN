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

/* Includes ------------------------------------------------------------------*/
#include "ramn_utils.h"
#include "usbd_gsusb_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/** @defgroup USBD_GSUSB_IF_Private_Variables USBD_GSUSB_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferGSUSB[GSUSB_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferGSUSB[GSUSB_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */



extern USBD_HandleTypeDef hUsbDeviceFS;


static int8_t GSUSB_Init(USBD_HandleTypeDef *pdev, osMessageQueueId_t q_frame_pool, osMessageQueueId_t q_from_host);
static int8_t GSUSB_DeInit(USBD_HandleTypeDef *pdev);
static int8_t GSUSB_Receive(USBD_HandleTypeDef *pdev, uint8_t* pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
#if 0
void RAMN_GSUSB_Init(StreamBufferHandle_t* pBuffer, osThreadId_t* pRecvTask, osThreadId_t* pSendTask)
{
	USBD_recvBuffer = pBuffer;
	USBD_recvTask = pRecvTask;
	USBD_sendTask = pSendTask;
}
#endif

uint8_t RAMN_GSUSB_GetTXStatus()
{
	USBD_Composite_HandleTypeDef *hcdc = (USBD_Composite_HandleTypeDef*)hUsbDeviceFS.pClassData;
	if (hcdc->TxState[1] != 0)
	{
		return USBD_BUSY;
	}
	else
	{
		return USBD_OK;
	}
}
/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

USBD_GSUSB_ItfTypeDef USBD_Interface_fops_GSUSB =
{
	GSUSB_Init,
	GSUSB_DeInit,
	GSUSB_Receive,
	GSUSB_Transmit
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t GSUSB_Init(USBD_HandleTypeDef *pdev, osMessageQueueId_t q_frame_pool, osMessageQueueId_t q_from_host)
{
	uint8_t ret = USBD_FAIL;
	USBD_GS_CAN_HandleTypeDef *hcan = calloc(1, sizeof(USBD_GS_CAN_HandleTypeDef));

	/* Set Application Buffers */
	USBD_Composite_SetTxBuffer(&hUsbDeviceFS, UserTxBufferGSUSB, 0, 1);
	USBD_Composite_SetRxBuffer(&hUsbDeviceFS, UserRxBufferGSUSB, 1);

	if(hcan != NULL)
	{
		hcan->q_frame_pool = q_frame_pool;
		hcan->q_recv_host = q_from_host;
		((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcan = hcan;
		hcan->from_host_buf = NULL;

		ret = USBD_OK;
	} else {
		((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcan = NULL;
	}

	return ret;
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t GSUSB_DeInit(USBD_HandleTypeDef *pdev)
{
	free(((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcan);
	pdev->ep_out[GSUSB_OUT_EP & 0xFU].is_used = 0;
	pdev->ep_in[GSUSB_IN_EP & 0xFU].is_used   = 0;

	return (USBD_OK);
}


/**
  * @brief  Data received over USB OUT endpoint are sent over GSUSB interface
  *         through this function.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t GSUSB_Receive(USBD_HandleTypeDef *pdev, uint8_t* Buf, uint32_t *Len)
{
    /* USER CODE BEGIN 6 */
	uint8_t                    retval = USBD_FAIL;
	BaseType_t                 ret;
	BaseType_t                 taskWoken;
	USBD_GS_CAN_HandleTypeDef *hcan = ((USBD_Composite_HandleTypeDef*)pdev->pClassData)->hcan;

	hcan->out_requests++;

	// HAL driver return wrong length when received multi frames..
	// so add the first frame length(64bytes) to rxlen.
	uint32_t rxlen = ((USBD_Composite_HandleTypeDef *)pdev->pClassData)->RxLength[1];
	if (rxlen >= (sizeof(struct gs_host_frame) - 4))
	{
		ret = xQueueReceiveFromISR(hcan->q_frame_pool, &hcan->from_host_buf, &taskWoken);
		if(ret != pdPASS)
		{
			retval = USBD_FAIL;
		}
		else
		{
			GSUSB_UnmarshalFrame(pdev, Buf, *Len, hcan->from_host_buf);
			xQueueSendToBackFromISR(hcan->q_recv_host, &hcan->from_host_buf, &taskWoken);
			retval = USBD_OK;
		}
	}
	USBD_Composite_ReceivePacket(pdev, GSUSB_OUT_EP);

	return retval;
    /* USER CODE END 6 */
}

/**
  * @brief  GSUSB_Transmit
  *         Data to send over USB IN endpoint are sent over GSUSB interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
int8_t GSUSB_Transmit(USBD_HandleTypeDef *pdev, uint8_t* Buf, uint16_t Len)
{
	USBD_Composite_HandleTypeDef *hcmp = (USBD_Composite_HandleTypeDef *)pdev->pClassData;

	if (hcmp == NULL) return USBD_FAIL;

	if (hcmp->TxState[1] == 0)
	{
		hcmp->TxState[1] = 1;
		USBD_LL_Transmit(pdev, GSUSB_IN_EP, Buf, Len);

		return USBD_OK;
	}
	else
	{
		return USBD_BUSY;
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
