/*
 * gs_usb_breq.h
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

#ifndef INC_BREQ_H_
#define INC_BREQ_H_


/*=============================================================================*
*                                Include Files                                 *
*==============================================================================*/

#include <stdlib.h>
#include <string.h>
#include "stm32l5xx_hal.h"

#include "ramn_config.h"
#include "../../../USBClass/Composite/gs_usb/gs_usb_fdcan.h"
#include "usbd_ctlreq.h"
#include "usbd_ioreq.h"
#include "usbd_desc.h"


/*=============================================================================*
*                                 functions                                    *
*==============================================================================*/

/*!
 * @brief Set identify flag from host
 * @param[in] hcan  The structure of CAN handle
 */
void breq_set_host_format(USBD_GS_CAN_HandleTypeDef *hcan);

/*!
 * @brief Set identify flag from host
 * @param[in] hcan  The structure of CAN handle
 * @param[in] req   The structure of USB setup request
 */
void breq_set_identify(USBD_GS_CAN_HandleTypeDef *hcan, USBD_SetupReqTypedef *req);

/*!
 * @brief init/deinit CAN state and set CAN mode
 * @param[in] hcan  The structure of CAN handle
 * @param[in] req   The structure of USB setup request
 */
void breq_set_mode(USBD_GS_CAN_HandleTypeDef *hcan, USBD_SetupReqTypedef *req);

/*!
 * @brief Set identify flag from host
 * @param[in] hcan  The structure of CAN handle
 * @param[in] req   The structure of USB setup request
 */
void breq_set_bittiming(USBD_GS_CAN_HandleTypeDef *hcan, USBD_SetupReqTypedef *req);

/*!
 * @brief Set data-bittiming data from host
 * @param[in] hcan  The structure of CAN handle
 * @param[in] req   The structure of USB setup request
 */
void breq_set_data_bittiming(USBD_GS_CAN_HandleTypeDef *hcan, USBD_SetupReqTypedef *req);

#endif /* INC_BREQ_H_ */
