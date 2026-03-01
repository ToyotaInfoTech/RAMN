/*
 * ramn_gsusb.h
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

// This Module is to implement GS USB (candlelight) features.
#ifndef INC_RAMN_GSUSB_H_
#define INC_RAMN_GSUSB_H_

#include "main.h"

#ifdef ENABLE_GSUSB

#include "gs_usb_fdcan.h"

// Forwards a CAN message to USB
RAMN_Result_t RAMN_GSUSB_ProcessRX(FDCAN_RxHeaderTypeDef *canRxHeader, uint8_t *canRxData);

// Forward a USB message to CAN
RAMN_Result_t RAMN_GSUSB_ProcessTX(FDCAN_TxHeaderTypeDef *canTxHeader, uint8_t *canRxData);

// Send a CAN error frame to USB
RAMN_Result_t RAMN_GSUSB_SendErrorFrame(const FDCAN_ProtocolStatusTypeDef *protocolStatus, const FDCAN_ErrorCountersTypeDef *errorCount, uint32_t err);

#endif

#endif /* INC_RAMN_GSUSB_H_ */
