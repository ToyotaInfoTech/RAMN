/*
 * usbd_gs_usb.c
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

/*
	The MIT License (MIT)
	Copyright (c) 2016 Hubert Denkmair
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
 */

#include "ramn_config.h"


#include "usbd_composite.h"
#include "usbd_ctlreq.h"
#include "usbd_gs_usb.h"
#include "usbd_gsusb_if.h"
#include "gs_usb_breq.h"

#ifdef ENABLE_GSUSB
static uint8_t gsusb_vendor_request(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

// device info
static const struct gs_device_config gscan_dconf = {
		0, // reserved 1
		0, // reserved 2
		0, // reserved 3
		0, // interface count (0=1, 1=2..)
		1, // software version
		2  // hardware version
};
#endif

// bit timing constraints
const struct gs_device_bt_const gscan_btconst = {
		GS_CAN_FEATURE_LISTEN_ONLY  // supported features
		| GS_CAN_FEATURE_LOOP_BACK
		| GS_CAN_FEATURE_HW_TIMESTAMP
		| GS_CAN_FEATURE_IDENTIFY
		| GS_CAN_FEATURE_USER_ID
		| GS_CAN_FEATURE_PAD_PKTS_TO_MAX_PKT_SIZE,
		48000000, // can timing base clock
		1,        // tseg1 min
		16,       // tseg1 max
		2,        // tseg2 min
		8,        // tseg2 max
		4,        // sjw max
		1,        // brp min
		512,      // brp_max
		1,        // brp increment;
};


/*  Microsoft Compatible ID Feature Descriptor  */
static const uint8_t USBD_MS_COMP_ID_FEATURE_DESC[] = {
		0x40, 0x00, 0x00, 0x00, /* length */
		0x00, 0x01,             /* version 1.0 */
		0x04, 0x00,             /* descr index (0x0004) */
		0x02,                   /* number of sections */
		0x00, 0x00, 0x00, 0x00, /* reserved */
		0x00, 0x00, 0x00,
		0x00,                   /* interface number */
		0x01,                   /* reserved */
		0x57, 0x49, 0x4E, 0x55, /* compatible ID ("WINUSB\0\0") */
		0x53, 0x42, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, /* sub-compatible ID */
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, /* reserved */
		0x00, 0x00,
		0x01,                   /* interface number */
		0x01,                   /* reserved */
		0x57, 0x49, 0x4E, 0x55, /* compatible ID ("WINUSB\0\0") */
		0x53, 0x42, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, /* sub-compatible ID */
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, /* reserved */
		0x00, 0x00
};

/* Microsoft Extended Properties Feature Descriptor */
static const uint8_t USBD_MS_EXT_PROP_FEATURE_DESC[] = {
		0x92, 0x00, 0x00, 0x00, /* length */
		0x00, 0x01,				/* version 1.0 */
		0x05, 0x00,             /* descr index (0x0005) */
		0x01, 0x00,             /* number of sections */
		0x88, 0x00, 0x00, 0x00, /* property section size */
		0x07, 0x00, 0x00, 0x00, /* property data type 7: Unicode REG_MULTI_SZ */
		0x2a, 0x00,				/* property name length */

		0x44, 0x00, 0x65, 0x00, /* property name "DeviceInterfaceGUIDs" */
		0x76, 0x00, 0x69, 0x00,
		0x63, 0x00, 0x65, 0x00,
		0x49, 0x00, 0x6e, 0x00,
		0x74, 0x00, 0x65, 0x00,
		0x72, 0x00, 0x66, 0x00,
		0x61, 0x00, 0x63, 0x00,
		0x65, 0x00, 0x47, 0x00,
		0x55, 0x00, 0x49, 0x00,
		0x44, 0x00, 0x73, 0x00,
		0x00, 0x00,

		0x50, 0x00, 0x00, 0x00, /* property data length */

		0x7b, 0x00, 0x63, 0x00, /* property name: "{c15b4308-04d3-11e6-b3ea-6057189e6443}\0\0" */
		0x31, 0x00, 0x35, 0x00,
		0x62, 0x00, 0x34, 0x00,
		0x33, 0x00, 0x30, 0x00,
		0x38, 0x00, 0x2d, 0x00,
		0x30, 0x00, 0x34, 0x00,
		0x64, 0x00, 0x33, 0x00,
		0x2d, 0x00, 0x31, 0x00,
		0x31, 0x00, 0x65, 0x00,
		0x36, 0x00, 0x2d, 0x00,
		0x62, 0x00, 0x33, 0x00,
		0x65, 0x00, 0x61, 0x00,
		0x2d, 0x00, 0x36, 0x00,
		0x30, 0x00, 0x35, 0x00,
		0x37, 0x00, 0x31, 0x00,
		0x38, 0x00, 0x39, 0x00,
		0x65, 0x00, 0x36, 0x00,
		0x34, 0x00, 0x34, 0x00,
		0x33, 0x00, 0x7d, 0x00,
		0x00, 0x00, 0x00, 0x00
};

// This must be available even when GS_USB is off due to potential caching
USBD_StatusTypeDef USBD_GSUSB_CustomDeviceRequest(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	if (req->bRequest == USBD_GS_CAN_VENDOR_CODE)
	{
		switch (req->wIndex)
		{
		case 0x0004:
			RAMN_memcpy(USBD_DescBuf, USBD_MS_COMP_ID_FEATURE_DESC, sizeof(USBD_MS_COMP_ID_FEATURE_DESC));
			USBD_CtlSendData(pdev, USBD_DescBuf, MIN(sizeof(USBD_MS_COMP_ID_FEATURE_DESC), req->wLength));
			return USBD_OK;

		case 0x0005:
			if (req->wValue==0) { // only return our GUID for interface #0
				RAMN_memcpy(USBD_DescBuf, USBD_MS_EXT_PROP_FEATURE_DESC, sizeof(USBD_MS_EXT_PROP_FEATURE_DESC));
				USBD_CtlSendData(pdev, USBD_DescBuf, MIN(sizeof(USBD_MS_EXT_PROP_FEATURE_DESC), req->wLength));
				return USBD_OK;
			}
			break;
		}
	}

	return USBD_FAIL;
}

#ifdef ENABLE_GSUSB

static uint8_t gsusb_dfu_request(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	USBD_GS_CAN_HandleTypeDef *hcan = ((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcan;
	switch (req->bRequest) {

	case 0: // DETACH request
		hcan->dfu_detach_requested = true;
		break;

	case 3: // GET_STATIS request
		hcan->ep0_buf[0] = 0x00; // bStatus: 0x00 == OK
		hcan->ep0_buf[1] = 0x00; // bwPollTimeout
		hcan->ep0_buf[2] = 0x00;
		hcan->ep0_buf[3] = 0x00;
		hcan->ep0_buf[4] = 0x00; // bState: appIDLE
		hcan->ep0_buf[5] = 0xFF; // status string descriptor index
		USBD_CtlSendData(pdev, hcan->ep0_buf, 6);
		break;

	default:
		USBD_CtlError(pdev, req);

	}
	return USBD_OK;
}

static uint8_t gsusb_config_request(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	USBD_GS_CAN_HandleTypeDef *hcan;
	uint8_t ret = USBD_OK;

	if ((USBD_Composite_HandleTypeDef *)pdev->pClassData == NULL)
	{
		return USBD_FAIL;
	}
	hcan =  ((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcan;

	switch (req->bRequest)
	{
	case GS_USB_BREQ_HOST_FORMAT:
	case GS_USB_BREQ_MODE:
	case GS_USB_BREQ_BITTIMING:
	case GS_USB_BREQ_IDENTIFY:

		hcan->last_setup_request = *req;
		USBD_CtlPrepareRx(pdev, hcan->ep0_buf, req->wLength);
		break;
	case GS_USB_BREQ_DATA_BITTIMING:

		hcan->enable_fdcan = 1;
		hcan->last_setup_request = *req;
		USBD_CtlPrepareRx(pdev, hcan->ep0_buf, req->wLength);
		break;
	case GS_USB_BREQ_DEVICE_CONFIG:
		RAMN_memcpy(hcan->ep0_buf, &gscan_dconf, sizeof(gscan_dconf));
		USBD_CtlSendData(pdev, hcan->ep0_buf, req->wLength);
		break;

	case GS_USB_BREQ_BT_CONST:
		RAMN_memcpy(hcan->ep0_buf, &gscan_btconst, sizeof(gscan_btconst));
		USBD_CtlSendData(pdev, hcan->ep0_buf, req->wLength);
		break;

	case GS_USB_BREQ_TIMESTAMP:
		RAMN_memcpy(hcan->ep0_buf, &hcan->sof_timestamp_us, sizeof(hcan->sof_timestamp_us));
		USBD_CtlSendData(pdev, hcan->ep0_buf, sizeof(hcan->sof_timestamp_us));
		break;

	default:
		//USBD_CtlError(pdev, req);
		ret = USBD_FAIL;
	}

	return ret;
}

static uint8_t gsusb_vendor_request(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	uint8_t req_rcpt = req->bRequest & 0x1F;
	uint8_t req_type = (req->bRequest >> 5) & 0x03;

	if (
			(req_type == 0x01) // class request
			&& (req_rcpt == 0x01) // recipient: interface
			&& (req->wIndex == DFU_INTERFACE_NUM)
	) {
		return gsusb_dfu_request(pdev, req);
	} else {
		return gsusb_config_request(pdev, req);
	}
}

USBD_StatusTypeDef USBD_GSUSB_Init(USBD_HandleTypeDef *pdev)
{
	USBD_StatusTypeDef ret;

	ret = USBD_LL_OpenEP(pdev, GSUSB_IN_EP, USBD_EP_TYPE_BULK, CAN_DATA_MAX_PACKET_SIZE);
	if(ret != USBD_OK)
	{
		return ret;
	}
	pdev->ep_in[GSUSB_IN_EP & 0xFU].is_used = 1U;

	USBD_LL_OpenEP(pdev, GSUSB_OUT_EP, USBD_EP_TYPE_BULK, CAN_DATA_MAX_PACKET_SIZE);
	if(ret != USBD_OK)
	{
		return ret;
	}
	pdev->ep_out[GSUSB_OUT_EP & 0xFU].is_used = 1U;

	((USBD_GSUSB_ItfTypeDef  *)pdev->pUserData[1])->Init(pdev, RAMN_GSUSB_PoolQueueHandle, RAMN_GSUSB_RecvQueueHandle);

	// Initialize can-fd instance for SocketCAN
	FDCAN_Instance *fdcanIns = FDCAN_GetInstance();
	FDCAN_DefaultInit(fdcanIns);
	USBD_GSUSB_SetChannel(pdev, 0, fdcanIns);

	((USBD_Composite_HandleTypeDef *)pdev->pClassData)->TxState[1] = 0U;
	((USBD_Composite_HandleTypeDef *)pdev->pClassData)->RxState[1] = 0U;

	return ret;
}

USBD_StatusTypeDef USBD_GSUSB_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	static uint8_t ifalt = 0;
	USBD_StatusTypeDef ret = USBD_OK;

	switch (req->bmRequest & USB_REQ_TYPE_MASK)
	{
	case USB_REQ_TYPE_CLASS:
	case USB_REQ_TYPE_VENDOR:
		return gsusb_vendor_request(pdev, req);

	case USB_REQ_TYPE_STANDARD:
		switch (req->bRequest)
		{
		case USB_REQ_GET_INTERFACE:
			USBD_CtlSendData(pdev, &ifalt, 1);
			break;

		case USB_REQ_SET_INTERFACE:
		default:
			break;
		}
		break;

		default:
			ret = USBD_FAIL;
			break;
	}

	return ret;
}


const struct gs_device_bt_const *USBD_GSUSB_Get_Bdconst(void)
{
	return &gscan_btconst;
}

USBD_StatusTypeDef USBD_GSUSB_Start(USBD_HandleTypeDef *pdev)
{
	USBD_StatusTypeDef         ret;

	ret = USBD_FAIL;
	if (pdev->pClassData)
	{
		//xQueueReceiveFromISR(hcan->q_frame_pool, &hcan->from_host_buf, &pxTaskWoken);
		USBD_Composite_ReceivePacket(pdev, GSUSB_OUT_EP);
		ret = USBD_OK;
	}
	else
	{
		ret = USBD_FAIL;
	}

	return ret;
}

USBD_StatusTypeDef USBD_GSUSB_SetChannel(USBD_HandleTypeDef *pdev, uint8_t channel, void* fdcanIns)
{
	USBD_StatusTypeDef         ret;
	USBD_GS_CAN_HandleTypeDef *hcan;

	hcan = ((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcan;
	if ((hcan != NULL) && (channel < NUM_CAN_CHANNEL))
	{
		hcan->channels[channel] = fdcanIns;
		ret = USBD_OK;
	}
	else ret = USBD_FAIL;

	return ret;
}


USBD_StatusTypeDef USBD_GSUSB_SendFrame(USBD_HandleTypeDef *pdev, struct gs_host_frame *frame)
{
	uint16_t                   len;
	uint8_t                    buf[128];
	USBD_GS_CAN_HandleTypeDef *hcan;

	hcan = ((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcan;

	GSUSB_MarshalFrame(pdev, frame, buf, &len);

	if(hcan->pad_pkts_to_max_pkt_size)
	{
		// When talking to WinUSB it seems to help a lot if the
		// size of packet you send equals the max packet size.
		// In this mode, fill packets out to max packet size and
		// then send.

		// zero rest of buffer
		RAMN_memset(buf + len, 0, sizeof(buf) - len);
		len = sizeof(buf);
	}

	return (USBD_StatusTypeDef)GSUSB_Transmit(pdev, buf, len);
}

void GSUSB_MarshalFrame(USBD_HandleTypeDef *pdev, struct gs_host_frame *in, uint8_t *out, uint16_t *outlen)
{
	uint16_t ofs;
	USBD_GS_CAN_HandleTypeDef *hcan;

	ofs = 0;
	hcan = ((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcan;

	RAMN_memcpy(&out[ofs], &in->echo_id, sizeof(in->echo_id)); ofs += sizeof(in->echo_id);
	RAMN_memcpy(&out[ofs], &in->can_id, sizeof(in->can_id)); ofs += sizeof(in->can_id);
	out[ofs++] = in->can_dlc;
	out[ofs++] = in->channel;
	out[ofs++] = in->flags;
	out[ofs++] = in->reserved;
	RAMN_memcpy(&out[ofs], in->data, in->can_dlc); ofs += in->can_dlc;
	if(!hcan->enable_fdcan) RAMN_memset(&out[ofs], 0x00, 8 - in->can_dlc);
	else RAMN_memset(&out[ofs], 0x00, 64 - in->can_dlc);

	if (hcan->timestamps_enabled)
	{
		RAMN_memcpy(&out[ofs], &in->timestamp_us, sizeof(in->timestamp_us)); ofs += sizeof(in->timestamp_us);
	}

	*outlen = ofs;
}

void GSUSB_UnmarshalFrame(USBD_HandleTypeDef *pdev, uint8_t *in, uint16_t inlen, struct gs_host_frame *out)
{
	uint16_t ofs;
	USBD_GS_CAN_HandleTypeDef *hcan;

	ofs = 0;
	hcan = ((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcan;

	RAMN_memcpy(&out->echo_id, &in[ofs], sizeof(out->echo_id)); ofs += sizeof(out->echo_id);
	RAMN_memcpy(&out->can_id, &in[ofs], sizeof(out->can_id)); ofs += sizeof(out->can_id);
	out->can_dlc = in[ofs++];
	out->channel = in[ofs++];
	out->flags = in[ofs++];
	out->reserved = in[ofs++];
	RAMN_memcpy(out->data, &in[ofs], out->can_dlc); ofs += out->can_dlc;
	if(!hcan->enable_fdcan) RAMN_memset(&out->data[ofs], 0x00, 8 - out->can_dlc);
	else RAMN_memset(&out->data[ofs], 0x00, 64 - out->can_dlc);

	if (hcan->timestamps_enabled)
	{
		RAMN_memcpy(&out->timestamp_us, &in[ofs], sizeof(out->timestamp_us));
	}
}

uint8_t GSUSB_IsConnected(USBD_HandleTypeDef *pdev)
{
	USBD_Composite_HandleTypeDef *hcmp;

	hcmp = (USBD_Composite_HandleTypeDef *)pdev->pClassData;
	if(hcmp->hcan == NULL) return 0;

	return hcmp->hcan->connected;
}

#endif
