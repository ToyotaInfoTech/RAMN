/*
 * gs_usb_breq.c
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

#include "stm32l5xx_hal.h"



#include "usbd_ctlreq.h"
#include "usbd_ioreq.h"
#include "usbd_desc.h"
#include "ramn_usb.h"

#ifdef ENABLE_GSUSB

#include "ramn_config.h"
#include "../../../USBClass/Composite/gs_usb/gs_usb_fdcan.h"
#include "../../../USBClass/Composite/gs_usb/usbd_gs_usb.h"

/*=============================================================================*
*                                 functions                                    *
*==============================================================================*/

/*!
 * @brief Set identify flag from host
 * @param[in] hcan  The structure of CAN handle
 */
void breq_set_host_format(USBD_GS_CAN_HandleTypeDef *hcan)
{
	RAMN_memcpy(&hcan->host_config, hcan->ep0_buf, sizeof(hcan->host_config));
}


/*!
 * @brief Set identify flag from host
 * @param[in] hcan  The structure of CAN handle
 * @param[in] req   The structure of USB setup request
 */
void breq_set_identify(USBD_GS_CAN_HandleTypeDef *hcan, USBD_SetupReqTypedef *req)
{
	int identify_flag;

	RAMN_memcpy(&identify_flag, hcan->ep0_buf, sizeof(identify_flag));
	if (identify_flag) {
		// Insert user code here when identify_flag == GS_CAN_IDENTIFY_ON
		// Example, at candlelight, run LED sequence.
	}
}


/*!
 * @brief init/deinit CAN state and set CAN mode
 * @param[in] hcan  The structure of CAN handle
 * @param[in] req   The structure of USB setup request
 */
void breq_set_mode(USBD_GS_CAN_HandleTypeDef *hcan, USBD_SetupReqTypedef *req)
{
#ifdef TARGET_ECUA
	struct gs_device_mode *mode;

	mode = (struct gs_device_mode*)hcan->ep0_buf;
	if (mode->mode == GS_CAN_MODE_RESET)
	{
		FDCAN_DeinitQueues();
		hcan->connected = 0;
		RAMN_USB_Config.gsusbOpened = 0;
	}
	else if (mode->mode == GS_CAN_MODE_START)
	{
#ifdef ENABLE_GSUSB_CANFD
		hcan->enable_fdcan = (mode->flags & GS_CAN_MODE_FD) != 0;
#else
		hcan->enable_fdcan = 0;
#endif
		FDCAN_InitQueues(hcan);
		hcan->timestamps_enabled = (mode->flags & GS_CAN_MODE_HW_TIMESTAMP) != 0;
		hcan->pad_pkts_to_max_pkt_size = (mode->flags & GS_CAN_MODE_PAD_PKTS_TO_MAX_PKT_SIZE) != 0;
		hcan->connected = 1;
		RAMN_USB_Config.gsusbOpened = 1;
	}
#endif
}


/*!
 * @brief Set bittiming data from host
 * @param[in] hcan  The structure of CAN handle
 * @param[in] req   The structure of USB setup request
 */
void breq_set_bittiming(USBD_GS_CAN_HandleTypeDef *hcan, USBD_SetupReqTypedef *req)
{
	struct gs_device_bittiming *timing;

	timing = (struct gs_device_bittiming *)hcan->ep0_buf;
	FDCAN_SetBittiming(
		hcan->channels[req->wValue],
		timing->brp,
		timing->prop_seg + timing->phase_seg1,
		timing->phase_seg2,
		timing->sjw
	);

}


/*!
 * @brief Set data-bittiming data from host
 * @param[in] hcan  The structure of CAN handle
 * @param[in] req   The structure of USB setup request
 */
void breq_set_data_bittiming(USBD_GS_CAN_HandleTypeDef *hcan, USBD_SetupReqTypedef *req)
{
#ifdef ENABLE_GSUSB_CANFD
	struct gs_device_bittiming *timing;

	timing = (struct gs_device_bittiming *)hcan->ep0_buf;
	FDCAN_SetDataBittiming(
		hcan->channels[req->wValue],
		timing->brp,
		timing->prop_seg + timing->phase_seg1,
		timing->phase_seg2,
		timing->sjw
	);

	hcan->enable_fdcan = 1;
#endif
}

#endif
