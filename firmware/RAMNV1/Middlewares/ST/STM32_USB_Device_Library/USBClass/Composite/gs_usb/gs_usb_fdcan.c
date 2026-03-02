/*
 * gs_usb_fdcan.c
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

#include "../../../USBClass/Composite/gs_usb/gs_usb_fdcan.h"

#include "cmsis_os.h"

#include "ramn_config.h"
#include "ramn_canfd.h"
#include "../../../USBClass/Composite/gs_usb/usbd_gs_usb.h"
#include "../../../USBClass/Composite/usbd_composite.h"

#ifdef ENABLE_GSUSB

/*=============================================================================*
 *                           Private variables                                  *
 *==============================================================================*/
static FDCAN_Instance        *fdcanIns;

// fdcan queue global variables

/** @brief  */
__attribute__ ((section (".buffers"))) static struct gs_host_frame   g_frames_list[CAN_QUEUE_SIZE];
static uint32_t g_frames_list_len;


/*=============================================================================*
 *                        Private function Prototypes                           *
 *==============================================================================*/

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/*!
 * @brief get fdcan instance
 * @param[in] void
 * @return fdcan_data_t structure pointer(global variable hCAN)
 */
FDCAN_Instance *FDCAN_GetInstance(void)
{
	if(fdcanIns == NULL)
	{
		fdcanIns = (FDCAN_Instance *)calloc(1, sizeof(FDCAN_Instance));
	}
	return fdcanIns;
}

/*!
 * @brief set fdcan instance
 * @param[in] fdcanIns_in fdcan instance[FDCAN_HandleTypeDef *]
 */
void FDCAN_SetInstance(FDCAN_HandleTypeDef *fdcanIns_in)
{
	if(fdcanIns == NULL)
	{
		fdcanIns = (FDCAN_Instance *)calloc(1, sizeof(FDCAN_Instance));
	}
	fdcanIns->instance = fdcanIns_in;
}

/*!
 * @brief set default value to fdcan_data_t structure
 * @param[in] fdcan_data     Can Instance
 * @return void
 */
void FDCAN_DefaultInit(FDCAN_Instance *fdcan_data)
{
	// default setting hfcan1 at main
	fdcan_data->instance = &hfdcan1;
	fdcan_data->bitrate_info.prescaler  = hfdcan1.Init.NominalPrescaler;
	fdcan_data->bitrate_info.phase_seg1 = hfdcan1.Init.NominalTimeSeg1;
	fdcan_data->bitrate_info.phase_seg2 = hfdcan1.Init.NominalTimeSeg2;
	fdcan_data->bitrate_info.sjw        = hfdcan1.Init.NominalSyncJumpWidth;

	fdcan_data->data_bitrate_info.prescaler  = hfdcan1.Init.DataPrescaler;
	fdcan_data->data_bitrate_info.phase_seg1 = hfdcan1.Init.DataPrescaler;
	fdcan_data->data_bitrate_info.phase_seg2 = hfdcan1.Init.DataPrescaler;
	fdcan_data->data_bitrate_info.sjw        = hfdcan1.Init.DataPrescaler;
}

/*!
 * @brief set CAN bittiming value to fdcan_data_t structure
 * the value is checked whether is valid by gs_device_bt_const
 * @param[in] fdcanHandle     FDCAN Instance
 * @param[in] nbrp            BRP value [Clock(48MHz) division]
 * @param[in] phase_seg1      TSEG1 value[prop_seg + pahse_seg1]
 * @param[in] phase_seg2      TSEG2 value
 * @param[in] sjw             Synchronization jump width value
 * @return true  : Success to set values
 *         false : failed to set values
 */
bool FDCAN_SetBittiming(FDCAN_Instance *fdcan_data, uint16_t nbrp, uint8_t phase_seg1, uint8_t phase_seg2, uint8_t sjw)
{
	//TODO: better implementation
	uint32_t baudrate = (48000000U)/(nbrp * (1U + phase_seg1 + phase_seg2));
	if(RAMN_FDCAN_UpdateBaudrate(baudrate) == RAMN_OK) return true;
	return false;
	/*
	 * 	const struct gs_device_bt_const *btconst = USBD_GSUSB_Get_Bdconst();
	 *
	if ( (phase_seg1 >= btconst->tseg1_min) && (phase_seg1 <= btconst->tseg1_max)
	  && (phase_seg2 >= btconst->tseg2_min) && (phase_seg2 <= btconst->tseg2_max)
	  && (sjw > 0) && (sjw <= btconst->sjw_max)
	) {
		fdcan_data->bitrate_info.prescaler = nbrp;
		fdcan_data->bitrate_info.phase_seg1 = phase_seg1;
		fdcan_data->bitrate_info.phase_seg2 = phase_seg2;
		fdcan_data->bitrate_info.sjw = sjw;
		return true;
	} else {
		return false;
	}*/
}

/*!
 * @brief set CAN data-bittiming value to fdcan_data_t structure
 * the value is checked whether is valid by gs_device_bt_const
 * @param[in] fdcanHandle     FDCAN Instance
 * @param[in] nbrp            BRP value [Clock(48MHz) division]
 * @param[in] phase_seg1      TSEG1 value[prop_seg + pahse_seg1]
 * @param[in] phase_seg2      TSEG2 value
 * @param[in] sjw             Synchronization jump width value
 * @return true  : Success to set values
 *         false : failed to set values
 */
bool FDCAN_SetDataBittiming(FDCAN_Instance *fdcan_data, uint16_t nbrp, uint8_t phase_seg1, uint8_t phase_seg2, uint8_t sjw)
{
	const struct gs_device_bt_const *btconst = USBD_GSUSB_Get_Bdconst();

	if ( (phase_seg1 >= btconst->tseg1_min) && (phase_seg1 <= btconst->tseg1_max)
			&& (phase_seg2 >= btconst->tseg2_min) && (phase_seg2 <= btconst->tseg2_max)
			&& (sjw > 0) && (sjw <= btconst->sjw_max)
	) {
		fdcan_data->data_bitrate_info.prescaler = nbrp;
		fdcan_data->data_bitrate_info.phase_seg1 = phase_seg1;
		fdcan_data->data_bitrate_info.phase_seg2 = phase_seg2;
		fdcan_data->data_bitrate_info.sjw = sjw;
		return true;
	} else {
		return false;
	}
}

/*!
 * @brief Initialize queues structure
 * @param[in] void
 * @return void
 */
void FDCAN_InitQueues(USBD_GS_CAN_HandleTypeDef *hcan)
{
	int      i;
	uint32_t addr;
	uint32_t size;

	if(g_frames_list_len == 0)
	{
		g_frames_list_len = CAN_QUEUE_SIZE / ((7 * hcan->enable_fdcan) + 1);
		size = GS_HOST_FRAME_SIZE - (56 * (hcan->enable_fdcan ^ 1));
		for(i = 0; i < g_frames_list_len; i++)
		{
			g_frames_list[i].data = calloc(1, size);
			if (g_frames_list[i].data == NULL)
			{
				Error_Handler();
			}
		}
	}
	if(uxQueueMessagesWaiting(RAMN_GSUSB_PoolQueueHandle) == 0)
	{
		for(i = 0; i < g_frames_list_len; i++)
		{
			addr = (uint32_t )&g_frames_list[i];
			// add frame to pool
			xQueueSendToBack(RAMN_GSUSB_PoolQueueHandle, &addr, portMAX_DELAY);
		}
	}
}

/*!
 * @brief deinitialize queues structure
 * @param[in] void
 * @return void
 */
void FDCAN_DeinitQueues(void)
{
	int i;

	for(i = 0; i < g_frames_list_len; i++)
	{
		free(g_frames_list[i].data);
	}
	g_frames_list_len = 0;
}

/*!
 * @brief get current queue max size
 * @retval current queue max size
 */
uint32_t FDCAN_GetQueueSize()
{
	return g_frames_list_len;
}

#endif
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
