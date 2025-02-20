/*
 * ramn_cdc.h
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

// This Module is to implement CDC (USB Serial) features
#ifndef INC_RAMN_CDC_H_
#define INC_RAMN_CDC_H_

#include "main.h"

#ifdef ENABLE_CDC

#include "ramn_usb.h"
#include "ramn_canfd.h"
#include "ramn_ecucontrol.h"
#include "ramn_memory.h"
#ifdef ENABLE_SCREEN
#include "ramn_screen_manager.h"
#endif
#ifdef ENABLE_MINICTF
#include "ramn_ctf.h"
#endif
#ifdef ENABLE_USB_DEBUG
#include "ramn_debug.h"
#endif
#ifdef ENABLE_UDS
#include "ramn_uds.h"
#endif

// Processes a CLI (custom line interface) command
RAMN_Bool_t RAMN_CDC_ProcessCLIBuffer(uint8_t* USBRxBuffer, uint32_t commandLength);

// Processes an slcan command
RAMN_Bool_t RAMN_CDC_ProcessSLCANBuffer(uint8_t* USBRxBuffer, uint32_t commandLength);

#endif
#endif /* INC_RAMN_CDC_H_ */
