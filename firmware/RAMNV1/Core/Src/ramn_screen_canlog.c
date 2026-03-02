/*
 * ramn_screen_canlog.c
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

#include "ramn_screen_canlog.h"

#ifdef ENABLE_SCREEN

__attribute__ ((section (".buffers"))) static volatile CAN_MessageBuffer canMessageBuffer = { .head = 0, .count = 0 };

// If you update this list, make sure you also update FILTER_MASK_LIST
const uint16_t FILTER_ID_LIST[]   = {
		CAN_SIM_CONTROL_BRAKE_CANID,
		CAN_SIM_CONTROL_ACCEL_CANID,
		CAN_SIM_CONTROL_STEERING_CANID,
		CAN_SIM_CONTROL_SHIFT_CANID,
		CAN_SIM_COMMAND_HORN_CANID,
		CAN_SIM_COMMAND_LIGHTS_CANID,
		CAN_SIM_COMMAND_TURNINDICATOR_CANID,
		CAN_SIM_CONTROL_ENGINEKEY_CANID,
		CAN_SIM_CONTROL_LIGHTS_CANID,
		CAN_SIM_CONTROL_SIDEBRAKE_CANID,
		0x550, // Assume XCP all start 0x55
		0x7E0, // Assume UDS all start 0x7E
		0x000
};

const uint16_t FILTER_MASK_LIST[] = {
		0x7FF,
		0x7FF,
		0x7FF,
		0x7FF,
		0x7FF,
		0x7FF,
		0x7FF,
		0x7FF,
		0x7FF,
		0x7FF,
		0x7F0,
		0x7F0,
		0x000};

// Index of filter currently used
static uint8_t filterIndex = 0U;

// Index of line being displayed on screen
static uint32_t dispIndex = 0U;

// Whether the screen is active or not
static RAMN_Bool_t active = True;

// Set to true if module had to give up writing on screen because it was slowing down RX processing
static RAMN_Bool_t overflowed = False;

// Semaphore to enable access from different tasks
static SemaphoreHandle_t CANLOG_SEMAPHORE = 0U;
static StaticSemaphore_t CANLOG_SEMAPHORE_STRUCT;

static void SCREENCANLOG_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick)
{
	volatile CAN_Message *message;

	if (CANLOG_SEMAPHORE == 0U) return; // Module is not initialized yet
	else if ((pHeader->IdType == FDCAN_STANDARD_ID) && (pHeader->FDFormat == FDCAN_CLASSIC_CAN) && (pHeader->RxFrameType == FDCAN_DATA_FRAME) && (pHeader->DataLength <= 8))
	{
		// Check filter
		if ((pHeader->Identifier & FILTER_MASK_LIST[filterIndex]) != (FILTER_ID_LIST[filterIndex] & FILTER_MASK_LIST[filterIndex])) return;

		// Take semaphore
		while(xSemaphoreTake(CANLOG_SEMAPHORE, portMAX_DELAY) != pdTRUE);

		message = &canMessageBuffer.messages[canMessageBuffer.head];
		message->identifier = pHeader->Identifier;
		message->payload_size = pHeader->DataLength;
		RAMN_memcpy((uint8_t*)message->data, data, pHeader->DataLength);

		// Update the buffer head and count
		canMessageBuffer.head = (canMessageBuffer.head + 1) % CAN_MESSAGE_BUFFER_SIZE;
		if (canMessageBuffer.count < CAN_MESSAGE_BUFFER_SIZE) canMessageBuffer.count++;

		// Give back Semaphore
		xSemaphoreGive(CANLOG_SEMAPHORE);
	}
}

static void draw_header()
{
	uint8_t tmp[8];
	uint12toASCII(FILTER_ID_LIST[filterIndex], tmp);
	tmp[3] = ':';
	uint12toASCII(FILTER_MASK_LIST[filterIndex], &tmp[4]);
	tmp[7] = 0; // Terminate string

	RAMN_SPI_DrawContour(0, 0, LCD_WIDTH, 16+6, CONTOUR_WIDTH, RAMN_SCREENUTILS_COLORTHEME.LIGHT);
	RAMN_SPI_RefreshString(5,5, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "RX DUMP");
	RAMN_SPI_RefreshString(5+8*11,5, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, (char*)tmp);

	if(active) RAMN_SPI_RefreshString(5+16*11,5, RAMN_SCREENUTILS_COLORTHEME.WHITE, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "  ON");
	else RAMN_SPI_RefreshString(5+16*11,5, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "STOP");
}

static void SCREENCANLOG_Init()
{
	canMessageBuffer.head = 0;
	canMessageBuffer.count = 0;
	if (CANLOG_SEMAPHORE == 0U) CANLOG_SEMAPHORE = xSemaphoreCreateMutexStatic(&CANLOG_SEMAPHORE_STRUCT);
	RAMN_SCREENUTILS_PrepareScrollScreen();
	dispIndex = 0;
	draw_header();
}

static void SCREENCANLOG_Update(uint32_t tick)
{
	if (overflowed)
	{
		if (dispIndex < CAN_MESSAGE_BUFFER_SIZE)
		{
			RAMN_SPI_RefreshString(9, CANVAS_OFFSET+(16*(dispIndex)), RAMN_SCREENUTILS_COLORTHEME.WHITE, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "Overflow            ");
		}
		else
		{
			RAMN_SPI_ScrollUp(16);
			RAMN_SPI_RefreshString(9, CANVAS_OFFSET+(16*(dispIndex%SCREEN_BUFFER_MESSAGE_COUNT)), RAMN_SCREENUTILS_COLORTHEME.WHITE, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "Overflow            ");
		}
		dispIndex += 1;
		overflowed = False;
	}
	if (xStreamBufferBytesAvailable(CANRxDataStreamBufferHandle) > MAX_BUFFER_BYTES)
	{
		// CAN messages available (skip until the buffer is empty)
		return;
	}
	if (RAMN_SCREENUTILS_LoopCounter % 5U == 0U)
	{
		// Too many messages received while drawing, write error and stop processing
		while (xSemaphoreTake(CANLOG_SEMAPHORE, portMAX_DELAY) != pdTRUE);

		if (active && (canMessageBuffer.count != 0U))
		{
			uint32_t start_index = (canMessageBuffer.head + CAN_MESSAGE_BUFFER_SIZE - canMessageBuffer.count) % CAN_MESSAGE_BUFFER_SIZE;
			for (uint8_t i = 0; i < canMessageBuffer.count; i++)
			{
				uint32_t index = (start_index + i) % CAN_MESSAGE_BUFFER_SIZE;
				// Safe to cast away volatile: access is protected by CANLOG_SEMAPHORE
				CAN_Message *message = (CAN_Message *)&canMessageBuffer.messages[index];
				uint8_t tmp[21];

				if(xStreamBufferBytesAvailable(CANRxDataStreamBufferHandle) > MAX_BUFFER_BYTES)
				{
					overflowed = True;
					break;
				}


				tmp[20] = 0;
				uint12toASCII(message->identifier, tmp);
				tmp[3] = ' ';
				for(uint8_t i=0;i<message->payload_size*2;i++)
				{
					uint4toASCII((message->data[i/2] >> (4*((i+1)%2)))&0xF,&tmp[4+i]);
				}
				tmp[4+message->payload_size*2] = 0;
				if (dispIndex < CAN_MESSAGE_BUFFER_SIZE)
				{
					RAMN_SPI_RefreshString(9, CANVAS_OFFSET+(16*(dispIndex)), RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, (char*)tmp);
					if (8 - message->payload_size > 0)
					{
						// Erase bytes after DLC (that could have been written by a previous message with longer DLC)
						RAMN_SPI_DrawRectangle(9+(4*11)+message->payload_size*22,CANVAS_OFFSET+(16*(dispIndex)),22*(8 - message->payload_size),14,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND);
					}
				}
				else
				{
					// Must scroll and display at last line
					RAMN_SPI_ScrollUp(16);
					RAMN_SPI_RefreshString(9, CANVAS_OFFSET+(16*(dispIndex%SCREEN_BUFFER_MESSAGE_COUNT)), RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, (char*)tmp);
					if (8 - message->payload_size > 0)
					{
						// Erase bytes after DLC (that could have been written by a previous message with longer DLC)
						RAMN_SPI_DrawRectangle(9+(4*11)+message->payload_size*22,CANVAS_OFFSET+(16*(dispIndex%SCREEN_BUFFER_MESSAGE_COUNT)),22*(8 - message->payload_size),14,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND);
					}
				}
				dispIndex += 1;
			}
			// Empty buffer
			canMessageBuffer.head = 0;
			canMessageBuffer.count = 0;
		}
		xSemaphoreGive(CANLOG_SEMAPHORE);
	}
}

static void SCREENCANLOG_Deinit()
{
	RAMN_SPI_SetScroll(SCREEN_HEADER_SIZE);
}

static RAMN_Bool_t SCREENCANLOG_UpdateInput(JoystickEventType event)
{
	if (event == JOYSTICK_EVENT_DOWN_PRESSED)
	{
		if (filterIndex == 0) filterIndex = (sizeof(FILTER_ID_LIST)/sizeof(uint16_t))-1;
		else filterIndex--;
		draw_header();
	}
	else if (event == JOYSTICK_EVENT_UP_PRESSED)
	{
		filterIndex = (filterIndex + 1) % (sizeof(FILTER_ID_LIST)/sizeof(uint16_t));
		draw_header();
	}
	else if (event == JOYSTICK_EVENT_CENTER_PRESSED)
	{
		active = !active;
		draw_header();
	}
	return True;
}

RAMNScreen_t ScreenCANLog = {
		.Init = SCREENCANLOG_Init,
		.Update = SCREENCANLOG_Update,
		.Deinit = SCREENCANLOG_Deinit,
		.UpdateInput = SCREENCANLOG_UpdateInput,
		.ProcessRxCANMessage = SCREENCANLOG_ProcessRxCANMessage
};

#endif
