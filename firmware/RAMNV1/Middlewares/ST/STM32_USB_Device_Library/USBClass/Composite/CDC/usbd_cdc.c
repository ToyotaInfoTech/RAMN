/*
 * usbd_cdc.c
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

/*=============================================================================*
*                                Include Files                                 *
*==============================================================================*/
#include "ramn_config.h"
#ifdef ENABLE_USB

#include "../../../USBClass/Composite/usbd_composite.h"
#include "usbd_ctlreq.h"
#include "usbd_cdc_if.h"
#include "../../../USBClass/Composite/CDC/usbd_cdc.h"


/*=============================================================================*
*                            Private Variables                                 *
*==============================================================================*/

/*=============================================================================*
*                            Public functions                                  *
*==============================================================================*/

static void cdc_init(USBD_HandleTypeDef *pdev, uint8_t index)
{
	if(index == 0)
	{
		/* Open EP IN */
		(void)USBD_LL_OpenEP(pdev, CDC_IN_EP, USBD_EP_TYPE_BULK, CDC_DATA_FS_IN_PACKET_SIZE);
		pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 1U;

		/* Open EP OUT */
		(void)USBD_LL_OpenEP(pdev, CDC_OUT_EP, USBD_EP_TYPE_BULK, CDC_DATA_FS_OUT_PACKET_SIZE);
		pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 1U;

		/* Set bInterval for CMD Endpoint */
		pdev->ep_in[CDC_CMD_EP & 0xFU].bInterval = CDC_FS_BINTERVAL;

		/* Open Command IN EP */
		(void)USBD_LL_OpenEP(pdev, CDC_CMD_EP, USBD_EP_TYPE_INTR, CDC_CMD_PACKET_SIZE);
		pdev->ep_in[CDC_CMD_EP & 0xFU].is_used = 1U;

		/* Init  physical Interface components */
		((USBD_CDC_ItfTypeDef *)pdev->pUserData[0])->Init();

		/* Init Xfer states */
		((USBD_Composite_HandleTypeDef *)pdev->pClassData)->TxState[0] = 0U;
		((USBD_Composite_HandleTypeDef *)pdev->pClassData)->RxState[0] = 0U;
	}
}

/**
  * @brief  USBD_CDC_Init
  *         Initialize CDC interface
  *
  * @param  pdev: USB device instance handle[USBD_HandleTypeDef *]
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
USBD_StatusTypeDef USBD_CDC_Init(USBD_HandleTypeDef *pdev)
{
	USBD_StatusTypeDef ret = USBD_OK;

#ifdef ENABLE_CDC
	cdc_init(pdev, 0);
#endif

#ifdef ENABLE_CDC
	USBD_Composite_ReceivePacket(pdev, CDC_OUT_EP);
#endif

	return ret;
}

/**
  * @brief  USBD_CDC_Setup
  *         Perform setup sequence for CDC interface
  *
  * @param  pdev: USB device instance handle[USBD_HandleTypeDef *]
  * @param  pdev: USB setup data [USBD_SetupReqTypedef *]
  * @param  index:CDC1 or CDC2 numbering
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
USBD_StatusTypeDef USBD_CDC_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req, uint8_t index)
{
	USBD_Composite_HandleTypeDef *hcmp = (USBD_Composite_HandleTypeDef *)pdev->pClassData;
	uint16_t len;
	uint8_t ifalt = 0U;
	uint16_t status_info = 0U;
	USBD_StatusTypeDef ret = USBD_OK;

	if (hcmp == NULL)
	{
		return (uint8_t)USBD_FAIL;
	}

	switch (req->bmRequest & USB_REQ_TYPE_MASK)
	{
	case USB_REQ_TYPE_CLASS:
		if (req->wLength != 0U)
		{
			if ((req->bmRequest & 0x80U) != 0U)
			{
				((USBD_CDC_ItfTypeDef *)pdev->pUserData[index])->Control(req->bRequest,
												(uint8_t *)hcmp->data,
												req->wLength);

				len = MIN(CDC_REQ_MAX_DATA_SIZE, req->wLength);
				(void)USBD_CtlSendData(pdev, (uint8_t *)hcmp->data, len);
			}
			else
			{
				hcmp->CmdOpCode = req->bRequest;
				hcmp->CmdLength = (uint8_t)req->wLength;

				(void)USBD_CtlPrepareRx(pdev, (uint8_t *)hcmp->data, req->wLength);
			}
		}
		else
		{
			((USBD_CDC_ItfTypeDef *)pdev->pUserData[index])->Control(req->bRequest,
											  (uint8_t *)req, 0U);
		}
		break;

	case USB_REQ_TYPE_STANDARD:
		switch (req->bRequest)
		{
		case USB_REQ_GET_STATUS:
			if (pdev->dev_state == USBD_STATE_CONFIGURED)
			{
				(void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
			}
			else
			{
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_GET_INTERFACE:
			if (pdev->dev_state == USBD_STATE_CONFIGURED)
			{
				(void)USBD_CtlSendData(pdev, &ifalt, 1U);
			}
			else
			{
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_SET_INTERFACE:
			if (pdev->dev_state != USBD_STATE_CONFIGURED)
			{
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_CLEAR_FEATURE:
			break;

		default:
			USBD_CtlError(pdev, req);
			ret = USBD_FAIL;
			break;
		}
		break;

	default:
		USBD_CtlError(pdev, req);
		ret = USBD_FAIL;
		break;
	}

	return (uint8_t)ret;
}

#endif

typedef int usbd_cdc_prevent_empty_translation_unit;
