/*
 * ramn_uart.h
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

// This module handles UART communications.

#ifndef INC_RAMN_UART_H_
#define INC_RAMN_UART_H_

#include "main.h"

#ifdef ENABLE_UART

#include "cmsis_os.h"
#include "task.h"
#include "semphr.h"
#include "stream_buffer.h"

// Initializes the module.
void 			RAMN_UART_Init(StreamBufferHandle_t* buffer,  osThreadId_t* pSendTask);

// Sends bytes over UART. Does not block (returns as soon as buffer is filled).
RAMN_Result_t 	RAMN_UART_SendFromTask(uint8_t* data, uint32_t length);

// Sends a string over serial UART.
RAMN_Result_t 	RAMN_UART_SendStringFromTask(const char* data);


#endif

#endif /* INC_RAMN_UART_H_ */
