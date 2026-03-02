/*
 * usbd_composite.c
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

/*=============================================================================*
*                                Include Files                                 *
*==============================================================================*/
#include "ramn_config.h"

#ifdef ENABLE_USB

#include "../../USBClass/Composite/usbd_composite.h"
#include "usbd_ctlreq.h"
#include "../../USBClass/Composite/CDC/usbd_cdc.h"
#include "usbd_cdc_if.h"
#include <usbd_gsusb_if.h>
#include "usbd_desc.h"
#include "../../USBClass/Composite/gs_usb/gs_usb_breq.h"
#include "../../USBClass/Composite/gs_usb/usbd_gs_usb.h"

/*=============================================================================*
*                       Static functions prototype                             *
*==============================================================================*/
static uint8_t  USBD_Composite_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_Composite_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_Composite_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t  USBD_Composite_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_Composite_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_Composite_EP0_RxReady(USBD_HandleTypeDef *pdev);

static uint8_t *USBD_Composite_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_Composite_GetDeviceQualifierDescriptor(uint16_t *length);
static uint8_t *USBD_Composite_GetStrdesc(USBD_HandleTypeDef *pdev, uint8_t index, uint16_t *length);

void (*USBD_errorCallback_ptr)(USBD_HandleTypeDef* hUsbDeviceFS);
void (*USBD_serialOpenCallback_ptr)(USBD_HandleTypeDef* hUsbDeviceFS, uint8_t index);
void (*USBD_serialCloseCallback_ptr)(USBD_HandleTypeDef* hUsbDeviceFS, uint8_t index);

/*=============================================================================*
*                            Static global variable                            *
*==============================================================================*/
/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_Composite_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
	USB_LEN_DEV_QUALIFIER_DESC,
	USB_DESC_TYPE_DEVICE_QUALIFIER,
	0x00,
	0x02,
	0x00,
	0x00,
	0x00,
	0x40,
	0x01,
	0x00,
};

/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_Composite_CfgFSDesc[] __ALIGN_END =
{
	//---------------------------------------------------------------------------
	// Configuration Descriptor
	//---------------------------------------------------------------------------
	0x09,                                 // bLength: Configuration Descriptor size
	USB_DESC_TYPE_CONFIGURATION,          // bDescriptorType: Configuration
	USB_COMPOSITE_CONFIG_DESC_SIZ,        // wTotalLength:no of returned bytes
	0x00,
	USBD_MAX_NUM_INTERFACES,              // bNumInterfaces: 2 or 4
	0x01,                                 // bConfigurationValue: Configuration value
	0x00,                                 // iConfiguration: Index of string descriptor describing the configuration
	0x80,                                 // bmAttributes: Bus powered
	0xFA,                                 // MaxPower 500 mA

#ifdef ENABLE_GSUSB
	//---------------------------------------------------------------------------
	// GS_USB Interface Descriptor
	//---------------------------------------------------------------------------
	0x09,                                 // bLength
	USB_DESC_TYPE_INTERFACE,              // bDescriptorType
	GSUSB_WINDEX,                         // bInterfaceNumber
	0x00,                                 // bAlternateSetting
	0x02,                                 // bNumEndpoints
	0xFF,                                 // bInterfaceClass: Vendor Specific
	0xFF,                                 // bInterfaceSubClass: Vendor Specific
	0xFF,                                 // bInterfaceProtocol: Vendor Specific
	0x00,                                 // iInterface
	//---------------------------------------------------------------------------

	//---------------------------------------------------------------------------
	// EP1 descriptor
	//---------------------------------------------------------------------------
	0x07,                                 // bLength
	USB_DESC_TYPE_ENDPOINT,               // bDescriptorType
	GSUSB_IN_EP,                          // bEndpointAddress
	0x02,                                 // bmAttributes: bulk
	LOBYTE(GSUSB_TX_DATA_SIZE),           // wMaxPacketSize
	HIBYTE(GSUSB_TX_DATA_SIZE),
	0x00,                                 // bInterval:
	//---------------------------------------------------------------------------

	//---------------------------------------------------------------------------
	// EP2 descriptor
	//---------------------------------------------------------------------------
	0x07,                                 // bLength
	USB_DESC_TYPE_ENDPOINT,               // bDescriptorType
	GSUSB_OUT_EP,                         // bEndpointAddress
	0x02,                                 // bmAttributes: bulk
	LOBYTE(GSUSB_RX_DATA_SIZE),           // wMaxPacketSize
	HIBYTE(GSUSB_RX_DATA_SIZE),
	0x00,                                 // bInterval:
	//---------------------------------------------------------------------------

	//---------------------------------------------------------------------------
	// DFU Interface Descriptor
	//---------------------------------------------------------------------------
	0x09,                                 // bLength
	USB_DESC_TYPE_INTERFACE,              // bDescriptorType
	GSUSB_WINDEX + 1,                     // bInterfaceNumber
	0x00,                                 // bAlternateSetting
	0x00,                                 // bNumEndpoints
	0xFE,                                 // bInterfaceClass: Vendor Specific
	0x01,                                 // bInterfaceSubClass
	0x01,                                 // bInterfaceProtocol : Runtime mode
	DFU_INTERFACE_STR_INDEX,              // iInterface

	//---------------------------------------------------------------------------
	// Run-Time DFU Functional Descriptor
	//---------------------------------------------------------------------------
	0x09,                                 // bLength
	0x21,                                 // bDescriptorType: DFU FUNCTIONAL
	0x0B,                                 // bmAttributes: detach, upload, download
	0xFF, 0x00,                           // wDetachTimeOut
	0x00, 0x64,                           // wTransferSize
	0x1a, 0x01,                           // bcdDFUVersion: 1.1a

#endif

#ifdef ENABLE_CDC
	//---------------------------------------------------------------------------
	// IAD for CDC class
	//---------------------------------------------------------------------------
	0x08,	                              // bLength: Interface Descriptor size
	0x0B,	                              // bDescriptorType: IAD
	CDC_WINDEX,	                          // bFirstInterface: starting of interface
	0x02,	                              // bInterfaceCount: interfaces under this IAD class
	0x02,	                              // bFunctionClass: CDC
	0x02,	                              // bFunctionSubClass
	0x01,	                              // bFunctionProtocol
	0x02,	                              // iFunction

	//---------------------------------------------------------------------------
	// Interface Descriptor(CDC)
	//---------------------------------------------------------------------------
	0x09,                                 // bLength: Interface Descriptor size
	USB_DESC_TYPE_INTERFACE,              // bDescriptorType: Interface
	CDC_WINDEX,                           // bInterfaceNumber: Number of Interface
	0x00,                                 // bAlternateSetting: Alternate setting
	0x01,                                 // bNumEndpoints: One endpoints used
	0x02,                                 // bInterfaceClass: Communication Interface Class
	0x02,                                 // bInterfaceSubClass: Abstract Control Model
	0x01,                                 // bInterfaceProtocol: Common AT commands
	0x00,                                 // iInterface:

	//---------------------------------------------------------------------------
	// Header Functional Descriptor
	//---------------------------------------------------------------------------
	0x05,                                 // bLength: Endpoint Descriptor size
	0x24,                                 // bDescriptorType: CS_INTERFACE
	0x00,                                 // bDescriptorSubtype: Header Func Desc
	0x10,                                 // bcdCDC: spec release number
	0x01,

	//---------------------------------------------------------------------------
	// Call Management Functional Descriptor
	//---------------------------------------------------------------------------
	0x05,                                 // bFunctionLength
	0x24,                                 // bDescriptorType: CS_INTERFACE
	0x01,                                 // bDescriptorSubtype: Call Management Func Desc
	0x00,                                 // bmCapabilities: D0+D1
	CDC_WINDEX + 1,                       // bDataInterface: 1

	//---------------------------------------------------------------------------
	// ACM Functional Descriptor
	//---------------------------------------------------------------------------
	0x04,                                 // bFunctionLength
	0x24,                                 // bDescriptorType: CS_INTERFACE
	0x02,                                 // bDescriptorSubtype: Abstract Control Management desc
	0x02,                                 // bmCapabilities

	//---------------------------------------------------------------------------
	// Union Functional Descriptor
	//---------------------------------------------------------------------------
	0x05,                                 // bFunctionLength
	0x24,                                 // bDescriptorType: CS_INTERFACE
	0x06,                                 // bDescriptorSubtype: Union func desc
	CDC_WINDEX,                           // bMasterInterface: Communication class interface
	CDC_WINDEX + 1,                       // bSlaveInterface0: Data Class Interface

	//---------------------------------------------------------------------------
	// Endpoint Descriptor(CDC_CMD:0x83)
	//---------------------------------------------------------------------------
	0x07,                                 // bLength: Endpoint Descriptor size
	USB_DESC_TYPE_ENDPOINT,               // bDescriptorType: Endpoint
	CDC_CMD_EP,                           // bEndpointAddress
	0x03,                                 // bmAttributes: Interrupt
	LOBYTE(CDC_CMD_PACKET_SIZE),          // wMaxPacketSize:
	HIBYTE(CDC_CMD_PACKET_SIZE),
	0x08,                                 // bInterval:
	//---------------------------------------------------------------------------

	//---------------------------------------------------------------------------
	// Data class interface descriptor
	//---------------------------------------------------------------------------
	0x09,                                 // bLength: Endpoint Descriptor size
	USB_DESC_TYPE_INTERFACE,              // bDescriptorType:
	CDC_WINDEX + 1,                       // bInterfaceNumber: Number of Interface
	0x00,                                 // bAlternateSetting: Alternate setting
	0x02,                                 // bNumEndpoints: Two endpoints used
	0x0A,                                 // bInterfaceClass: CDC
	0x00,                                 // bInterfaceSubClass:
	0x00,                                 // bInterfaceProtocol:
	0x00,                                 // iInterface:

	//---------------------------------------------------------------------------
	// Endpoint OUT Descriptor
	//---------------------------------------------------------------------------
	0x07,                                 // bLength: Endpoint Descriptor size
	USB_DESC_TYPE_ENDPOINT,               // bDescriptorType: Endpoint
	CDC_OUT_EP,                           // bEndpointAddress
	0x02,                                 // bmAttributes: Bulk
	LOBYTE(CDC_DATA_FS_OUT_PACKET_SIZE),  // wMaxPacketSize:
	HIBYTE(CDC_DATA_FS_OUT_PACKET_SIZE),
	0x00,                                 // bInterval: ignore for Bulk transfer

	//---------------------------------------------------------------------------
	// Endpoint IN Descriptor
	//---------------------------------------------------------------------------
	0x07,                                 // bLength: Endpoint Descriptor size
	USB_DESC_TYPE_ENDPOINT,               // bDescriptorType: Endpoint
	CDC_IN_EP,                            // bEndpointAddress
	0x02,                                 // bmAttributes: Bulk
	LOBYTE(CDC_DATA_FS_IN_PACKET_SIZE),   // wMaxPacketSize:
	HIBYTE(CDC_DATA_FS_IN_PACKET_SIZE),
	0x00,                                 // bInterval: ignore for Bulk transfer
#endif

};

// Microsoft OS String Descriptor
static __ALIGN_BEGIN uint8_t usbd_gscan_winusb_str[] __ALIGN_END =
{
	0x12,                    // length
	0x03,                    // descriptor type == string
	0x4D, 0x00, 0x53, 0x00,  // signature: "MSFT100"
	0x46, 0x00, 0x54, 0x00,
	0x31, 0x00, 0x30, 0x00,
	0x30, 0x00,
	USBD_GS_CAN_VENDOR_CODE, // vendor code
	0x00                     // padding
};

/*=============================================================================*
*                            Public global variable                            *
*==============================================================================*/
/* CDC interface class callbacks structure */
USBD_ClassTypeDef  USBD_Composite =
{
	USBD_Composite_Init,
	USBD_Composite_DeInit,
	USBD_Composite_Setup,
	NULL,                 /* EP0_TxSent, */
	USBD_Composite_EP0_RxReady,
	USBD_Composite_DataIn,
	USBD_Composite_DataOut,
	NULL,
	NULL,
	NULL,
	USBD_Composite_GetFSCfgDesc,
	USBD_Composite_GetFSCfgDesc,
	USBD_Composite_GetFSCfgDesc,
	USBD_Composite_GetDeviceQualifierDescriptor,
	USBD_Composite_GetStrdesc
};


/*=============================================================================*
*                               Static functions                               *
*==============================================================================*/
/**
  * @brief  USBD_CDC_Init
  *         Initialize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_Composite_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	UNUSED(cfgidx);
	uint8_t                        ret;
	USBD_Composite_HandleTypeDef  *hcmp_composite;

	hcmp_composite = USBD_malloc(sizeof(USBD_Composite_HandleTypeDef));
	if (hcmp_composite == NULL)
	{
		pdev->pClassData = NULL;
		return (uint8_t)USBD_EMEM;
	}

	pdev->pClassData = (void *)hcmp_composite;

	ret = USBD_CDC_Init(pdev);
	if(ret != USBD_OK) return ret;

#ifdef ENABLE_GSUSB
	ret = USBD_GSUSB_Init(pdev);
	if(ret != USBD_OK) return ret;
	ret = USBD_GSUSB_Start(pdev);
#endif

	return ret;
}

/**
  * @brief  USBD_CDC_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_Composite_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	UNUSED(cfgidx);

#ifdef ENABLE_CDC
	/* Close EP IN */
	(void)USBD_LL_CloseEP(pdev, CDC_IN_EP);
	pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 0U;

	/* Close EP OUT */
	(void)USBD_LL_CloseEP(pdev, CDC_OUT_EP);
	pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 0U;

	/* Close Command IN EP */
	(void)USBD_LL_CloseEP(pdev, CDC_CMD_EP);
	pdev->ep_in[CDC_CMD_EP & 0xFU].is_used = 0U;
	pdev->ep_in[CDC_CMD_EP & 0xFU].bInterval = 0U;
#endif
#ifdef ENABLE_GSUSB
	/* Close EP IN */
	(void)USBD_LL_CloseEP(pdev, GSUSB_IN_EP);
	pdev->ep_in[GSUSB_IN_EP & 0xFU].is_used = 0U;

	/* Close EP OUT */
	(void)USBD_LL_CloseEP(pdev, GSUSB_OUT_EP);
	pdev->ep_out[GSUSB_OUT_EP & 0xFU].is_used = 0U;
#endif

	/* DeInit  physical Interface components */
	if (pdev->pClassData != NULL)
	{
#ifdef ENABLE_CDC
		((USBD_CDC_ItfTypeDef *)pdev->pUserData[0])->DeInit();
#endif
#ifdef ENABLE_GSUSB
		((USBD_GSUSB_ItfTypeDef *)pdev->pUserData[1])->DeInit(pdev);
#endif
		(void)USBD_free(pdev->pClassData);
		pdev->pClassData = NULL;
	}

	return (uint8_t)USBD_OK;
}



/**
  * @brief  USBD_CDC_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_Composite_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	USBD_StatusTypeDef            ret = USBD_OK;

	//if(req->wIndex == 4U) req->wIndex = 2;
	switch(req->wIndex)
	{
#ifdef ENABLE_GSUSB
	case GSUSB_WINDEX:
		ret = USBD_GSUSB_Setup(pdev, req);
		break;
#endif
#ifdef ENABLE_CDC
	case CDC_WINDEX:
		ret = USBD_CDC_Setup(pdev, req, 0);
		break;
//	case 0x04:
//		ret = USBD_OK; //
#endif
	default:
		//Error_Handler();
		break;
	}

	if(ret != USBD_OK)
	{
		USBD_CtlError(pdev, req);
	}

	return (uint8_t)ret;
}



/**
  * @brief  USBD_CDC_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_Composite_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	USBD_Composite_HandleTypeDef *hcmp;
	PCD_HandleTypeDef *hpcd = pdev->pData;

	if (pdev->pClassData == NULL)
	{
		return (uint8_t)USBD_FAIL;
	}

	if ((pdev->ep_in[epnum].total_length > 0U) &&
	   ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U))
	{
		/* Update the packet total length */
		pdev->ep_in[epnum].total_length = 0U;
		/* Send ZLP */
		(void)USBD_LL_Transmit(pdev, epnum, NULL, 0U);
	}
	else
	{
		hcmp = (USBD_Composite_HandleTypeDef *)pdev->pClassData;
		if ((USBD_CDC_ItfTypeDef *)pdev->pUserData[0] != NULL && epnum == (CDC_IN_EP & 0x7F))
		{
			hcmp->TxState[0] = 0U;
			((USBD_CDC_ItfTypeDef *)pdev->pUserData[0])->TransmitCplt(hcmp->TxBuffer[0], &hcmp->TxLength[0], epnum);
		}
		else if ((USBD_GSUSB_ItfTypeDef *)pdev->pUserData[1] != NULL && epnum == (GSUSB_IN_EP & 0x7F))
		{
			hcmp->TxState[1] = 0U;
		}
	}

	return (uint8_t)USBD_OK;
}



/**
  * @brief  USBD_CDC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_Composite_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	USBD_Composite_HandleTypeDef *hcmp = (USBD_Composite_HandleTypeDef *)pdev->pClassData;

	if (pdev->pClassData == NULL)
	{
		return (uint8_t)USBD_FAIL;
	}

	/* Get the received data length */
	if(epnum == (CDC_OUT_EP & 0x7F)) hcmp->RxLength[0] = USBD_LL_GetRxDataSize(pdev, epnum);
	else if(epnum == (GSUSB_OUT_EP & 0x7F)) hcmp->RxLength[1] = USBD_LL_GetRxDataSize(pdev, epnum);
	else hcmp->RxLength[2] = USBD_LL_GetRxDataSize(pdev, epnum);

	/* USB data will be immediately processed, this allow next USB traffic being
	NAKed till the end of the application Xfer */
	if((USBD_CDC_ItfTypeDef *)pdev->pUserData[0] != NULL && epnum == (CDC_OUT_EP & 0x7F))
	{
		((USBD_CDC_ItfTypeDef *)pdev->pUserData[0])->Receive(hcmp->RxBuffer[0], &hcmp->RxLength[0]);
	}
	else if((USBD_GSUSB_ItfTypeDef *)pdev->pUserData[1] != NULL && epnum == (GSUSB_OUT_EP & 0x7F))
	{
		((USBD_GSUSB_ItfTypeDef *)pdev->pUserData[1])->Receive(pdev, hcmp->RxBuffer[1], &hcmp->RxLength[1]);
	}

	return (uint8_t)USBD_OK;
}



/**
  * @brief  USBD_Composite_EP0_RxReady
  *         Handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_Composite_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
	USBD_Composite_HandleTypeDef *hcmp = (USBD_Composite_HandleTypeDef *)pdev->pClassData;
#ifdef ENABLE_GSUSB
	USBD_SetupReqTypedef         *req;
	uint32_t                      param_u32;
#endif

	if (hcmp == NULL)
	{
		return (uint8_t)USBD_FAIL;
	}


#ifdef ENABLE_GSUSB
	req = &hcmp->hcan->last_setup_request;

	if(req->bRequest == GS_USB_BREQ_HOST_FORMAT && hcmp->hcan->ep0_buf[0] != 0x00)
	{
		// Ignore  value (Linux kernel driver sends 0x0000beef)
		RAMN_memcpy(&hcmp->hcan->host_config, hcmp->hcan->ep0_buf, sizeof(hcmp->hcan->host_config));
	}
	else
	{
		switch (req->bRequest)
		{
		case GS_USB_BREQ_IDENTIFY:
			RAMN_memcpy(&param_u32, hcmp->hcan->ep0_buf, sizeof(param_u32));
			break;

		case GS_USB_BREQ_MODE:
			if (req->wValue < NUM_CAN_CHANNEL)
			{
				breq_set_mode(hcmp->hcan, req);
			}
			break;

		case GS_USB_BREQ_BITTIMING:
			if (req->wValue < NUM_CAN_CHANNEL)
			{
				breq_set_bittiming(hcmp->hcan, req);
			}
			break;
		case GS_USB_BREQ_DATA_BITTIMING:
			if (req->wValue < NUM_CAN_CHANNEL)
			{
				breq_set_data_bittiming(hcmp->hcan, req);
			}
			break;
		default:
			// EP0 for CDC interface
			if (hcmp->CmdOpCode != 0xFFU)
			{
				// CDC
				((USBD_CDC_ItfTypeDef *)pdev->pUserData[0])->Control(hcmp->CmdOpCode,
																	(uint8_t *)hcmp->data,
																	(uint16_t)hcmp->CmdLength);
				hcmp->CmdOpCode = 0xFFU;
			}
			break;
		}
	}
#else
	// EP0 for CDC interface
	if ((pdev->pUserData[0] != NULL) && (hcmp->CmdOpCode != 0xFFU))
	{
		((USBD_CDC_ItfTypeDef *)pdev->pUserData[0])->Control(hcmp->CmdOpCode,
															(uint8_t *)hcmp->data,
															(uint16_t)hcmp->CmdLength);

		hcmp->CmdOpCode = 0xFFU;
	}
#endif

	return (uint8_t)USBD_OK;
}



/**
  * @brief  USBD_Composite_GetFSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_Composite_GetFSCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_Composite_CfgFSDesc);

  return USBD_Composite_CfgFSDesc;
}



/**
  * @brief  USBD_Composite_GetDeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_Composite_GetDeviceQualifierDescriptor(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_Composite_DeviceQualifierDesc);

  return USBD_Composite_DeviceQualifierDesc;
}


/**
  * @brief  USBD_Composite_GetStrdesc
  *         return Device Strings descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_Composite_GetStrdesc(USBD_HandleTypeDef *pdev, uint8_t index, uint16_t *length)
{
	UNUSED(pdev);

	switch (index) {
		case DFU_INTERFACE_STR_INDEX:
			USBD_GetString(GSUSB_DFU_INTERFACE_STRING_FS, USBD_StrDesc, length);
			return USBD_StrDesc;
		case 0xEE:
			*length = sizeof(usbd_gscan_winusb_str);
			return usbd_gscan_winusb_str;
		default:
			*length = 0;
			USBD_CtlError(pdev, 0);
			return 0;
	}
}

/*=============================================================================*
*                               Public functions                               *
*==============================================================================*/
/**
  * @brief  USBD_Composite_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @param  idx 0: for CDC
  *             1: for gs_usb
  * @retval status
  */
uint8_t USBD_Composite_RegisterInterface(
	USBD_HandleTypeDef  *pdev,
    void                *fops,
	uint32_t             idx
)
{
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  pdev->pUserData[idx] = fops;

  return (uint8_t)USBD_OK;
}



/**
  * @brief  USBD_Composite_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @param  length Tx buffer length
  * @param  idx 0: for CDC
  *             1: for gs_usb
  * @retval status
  */
uint8_t USBD_Composite_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint32_t length, uint8_t idx)
{
	USBD_Composite_HandleTypeDef *hcmp = (USBD_Composite_HandleTypeDef *)pdev->pClassData;
	if (hcmp == NULL)
	{
		return (uint8_t)USBD_FAIL;
	}

	hcmp->TxBuffer[idx] = pbuff;
	hcmp->TxLength[idx] = length;

	return (uint8_t)USBD_OK;
}



/**
  * @brief  USBD_Composite_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @param  idx 0: for CDC
  *             1: for gs_usb
  * @retval status
  */
uint8_t USBD_Composite_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint8_t idx)
{
	USBD_Composite_HandleTypeDef *hcmp = (USBD_Composite_HandleTypeDef *)pdev->pClassData;

	if (hcmp == NULL)
	{
		return (uint8_t)USBD_FAIL;
	}

	hcmp->RxBuffer[idx] = pbuff;

	return (uint8_t)USBD_OK;
}



/**
  * @brief  USBD_Composite_TransmitPacket
  *         Transmit packet on IN endpoint
  * @param  pdev: device instance
  * @param  idx 0: transmit for CDC
  *             1: transmit for gs_usb
  * @retval status
  */
uint8_t USBD_Composite_TransmitPacket(USBD_HandleTypeDef *pdev, uint8_t idx)
{
	USBD_Composite_HandleTypeDef *hcmp = (USBD_Composite_HandleTypeDef *)pdev->pClassData;
	USBD_StatusTypeDef ret = USBD_BUSY;

	if (pdev->pClassData == NULL) return (uint8_t)USBD_FAIL;

	if (hcmp->TxState[idx] == 0U)
	{
		/* Tx Transfer in progress */
		hcmp->TxState[idx] = 1U;

		/* Update the packet total length */
		if(idx == 0) pdev->ep_in[CDC_IN_EP & 0xFU].total_length = hcmp->TxLength[idx];
		else if(idx == 1) pdev->ep_in[GSUSB_IN_EP & 0xFU].total_length = hcmp->TxLength[idx];

		/* Transmit next packet */
		if(idx == 0)
		{
			(void)USBD_LL_Transmit(pdev, CDC_IN_EP, hcmp->TxBuffer[idx], hcmp->TxLength[idx]);
		}
		else if(idx == 1)
		{
			(void)USBD_LL_Transmit(pdev, GSUSB_IN_EP, hcmp->TxBuffer[idx], hcmp->TxLength[idx]);
		}

		ret = USBD_OK;
	}

	return (uint8_t)ret;
}



/**
  * @brief  USBD_Composite_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @param  epnum endpoint number
  * @retval status
  */
uint8_t USBD_Composite_ReceivePacket(USBD_HandleTypeDef *pdev, uint32_t epnum)
{
	USBD_Composite_HandleTypeDef *hcmp = (USBD_Composite_HandleTypeDef *)pdev->pClassData;
	uint8_t                      *buf;

	if (pdev->pClassData == NULL)
	{
		return (uint8_t)USBD_FAIL;
	}
	if(epnum == (CDC_OUT_EP & 0x7F))
	{
		buf = hcmp->RxBuffer[0];
		/* Prepare Out endpoint to receive next packet */
		(void)USBD_LL_PrepareReceive(pdev, epnum, buf, CDC_DATA_FS_OUT_PACKET_SIZE);
	}
	else if(epnum == (GSUSB_OUT_EP & 0x7F))
	{
		buf = hcmp->RxBuffer[1];
		/* Prepare Out endpoint to receive next packet */
		(void)USBD_LL_PrepareReceive(pdev, epnum, buf, GSUSB_RX_DATA_SIZE);
	}

	return (uint8_t)USBD_OK;
}

#endif

typedef int usbd_composite_prevent_empty_translation_unit;

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
