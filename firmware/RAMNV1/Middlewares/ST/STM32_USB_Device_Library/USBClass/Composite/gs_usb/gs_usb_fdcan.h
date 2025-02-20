/*
 * gs_usb_fdcan.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GS_USB_FDCAN_H
#define __GS_USB_FDCAN_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdbool.h>
#include "stm32l5xx_hal.h"
#include "../../../USBClass/Composite/gs_usb/usbd_gs_usb.h"
/* USER CODE END Includes */

extern FDCAN_HandleTypeDef hfdcan1;

/* USER CODE BEGIN Private defines */

// CAN function ID(sample)
#define FUNCID_SET_OFF_ECU_KEEPALIVE    0x8000
#define FUNCID_SET_ON_ECU_KEEPALIVE     0x8001
#define FUNCID_SET_ECU_BITRATE          0x8080
#define FUNCID_ECU_BOOT_COMPLETED       0x80FF
// add here CAN message function id
// #define FUNCID_xxx

// error code
#define FDCAN_OK               0
#define FDCAN_ERROR_QUEUE      1
#define FDCAN_ERROR_BAD_SIZE   2
#define FDCAN_ERROR_BAD_FUNCID 3
// add here CAN message error code
//#define FDCAN_ERROR_xxx

struct fdcan_bitrate_info
{
	uint8_t prescaler;
	uint8_t phase_seg1;
	uint8_t phase_seg2;
	uint8_t sjw;
};

typedef struct {
	FDCAN_HandleTypeDef       *instance;
	struct fdcan_bitrate_info  bitrate_info;
	struct fdcan_bitrate_info  data_bitrate_info;
} FDCAN_Instance;


/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */

/*!
 * @brief get fdcan instance
 * @param[in] void
 * @return fdcan_data_t structure pointer(global variable hCAN)
 */
FDCAN_Instance *FDCAN_GetInstance(void);

/*!
 * @brief set fdcan instance
 * @param[in] fdcanIns_in fdcan instance[FDCAN_HandleTypeDef *]
 */
void FDCAN_SetInstance(FDCAN_HandleTypeDef *fdcanIns_in);

/*!
 * @brief set default value to fdcan_data_t structure
 * @param[in] hfdcan     Can Instance
 * @param[in] RxFifo0ITs
 * @return void
 */
void FDCAN_DefaultInit(FDCAN_Instance *fdcan_data);

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
bool FDCAN_SetBittiming(FDCAN_Instance *fdcan_data, uint16_t nbrp, uint8_t phase_seg1, uint8_t phase_seg2, uint8_t sjw);

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
bool FDCAN_SetDataBittiming(FDCAN_Instance *fdcan_data, uint16_t nbrp, uint8_t phase_seg1, uint8_t phase_seg2, uint8_t sjw);

/*!
 * @brief Initialize queues structure
 * @param[in] void
 * @return void
 */
void FDCAN_InitQueues(USBD_GS_CAN_HandleTypeDef *hcan);

/*!
 * @brief deinitialize queues structure
 * @param[in] void
 * @return void
 */
void FDCAN_DeinitQueues(void);

/*!
 * @brief get current queue max size
 * @retval current queue max size
 */
uint32_t FDCAN_GetQueueSize(void);

/*!
 * @brief get can dlc associated HAL macro value
 * @param[in] actual byte length
 * @retval FDCAN_DLC_BYTES_xx(see HAL macro stm32l5xx_hal_fdcan.h)
 */
uint32_t FDCAN_ConvertToDLC(int n);

/*!
 * @brief get actual byte length from HAL macro value (see HAL macro stm32l5xx_hal_fdcan.h)
 * @param[in] HAL macro value
 * @retval actual byte length
 */
uint8_t FDCAN_ConvertToActual(uint32_t dlc);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__GS_USB_FDCAN_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
