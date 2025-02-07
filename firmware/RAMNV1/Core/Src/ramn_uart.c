/*
 * ramn_uart.c
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

#include "ramn_uart.h"

#ifdef ENABLE_UART

#include "string.h"

// Buffer that holds outgoing UART data
static StreamBufferHandle_t* uartTxBuffer;

// Pointer to the task currently responsible for sending out UART data
static osThreadId_t* sendTask;

// Semaphore to allow writing to UART from different task
static StaticSemaphore_t UART_TX_SEMAPHORE_STRUCT;
static SemaphoreHandle_t UART_TX_SEMAPHORE;

void RAMN_UART_Init(StreamBufferHandle_t* buffer,  osThreadId_t* pSendTask)
{
	uartTxBuffer 					= buffer;
	sendTask 						= pSendTask;
	UART_TX_SEMAPHORE 				= xSemaphoreCreateMutexStatic(&UART_TX_SEMAPHORE_STRUCT);
}

RAMN_Result_t RAMN_UART_SendFromTask(uint8_t* data, uint32_t length)
{
	size_t xBytesSent = 0;
	RAMN_Result_t result = RAMN_OK;

	while (xSemaphoreTake(UART_TX_SEMAPHORE, portMAX_DELAY ) != pdTRUE);
	xBytesSent = xStreamBufferSend(*uartTxBuffer, data, length, 2000U);
	xSemaphoreGive(UART_TX_SEMAPHORE);
	if (xBytesSent != length)
	{
		result = RAMN_ERROR;
		while(xStreamBufferReset(*uartTxBuffer) != pdPASS) osDelay(10U); //Overflow, clear buffer
	}

	return result;
}

RAMN_Result_t RAMN_UART_SendStringFromTask(char* data)
{
	return RAMN_UART_SendFromTask((uint8_t*)data, strlen(data));
}

#endif

