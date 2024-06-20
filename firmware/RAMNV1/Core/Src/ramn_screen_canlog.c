/*
 * ramn_screen_canlog.c
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2024 TOYOTA MOTOR CORPORATION.
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

CAN_MessageBuffer canMessageBuffer = { .head = 0, .count = 0 };

const uint16_t FILTER_ID_LIST[]   = {0x024, 0x039, 0x062, 0x077, 0x098, 0x150, 0x1A7, 0x1B8, 0x1BB, 0x1D3, 0x7E0, 0x000};
const uint16_t FILTER_MASK_LIST[] = {0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7FF, 0x7F0, 0x000};

#define NUMBER_OF_FILTERS (sizeof(FILTER_ID_LIST)/sizeof(uint16_t))

#define DEFAULT_FILTER_INDEX 0
uint8_t filter_index = DEFAULT_FILTER_INDEX;

static uint32_t disp_index = 0;
static uint8_t active = 1U;
static uint16_t filter_id = FILTER_ID_LIST[DEFAULT_FILTER_INDEX];
static uint16_t filter_mask = FILTER_MASK_LIST[DEFAULT_FILTER_INDEX];

////Semaphore to enable access from different threads
static SemaphoreHandle_t CANLOG_SEMAPHORE = 0U;
static StaticSemaphore_t CANLOG_SEMAPHORE_STRUCT;

// Function to set CAN filter
static void setCANfilterIndex(uint8_t index) {
	filter_id = FILTER_ID_LIST[filter_index];
	filter_mask = FILTER_MASK_LIST[filter_index];
}

void RAMN_ScreenCANLog_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick)
{
	while (CANLOG_SEMAPHORE == 0U) osDelay(50);
	if (xStreamBufferBytesAvailable(CANRxDataStreamBufferHandle) > MAX_BUFFER_BYTES)
	{
		//Too many CAN messages available (would be immediately overwritten, skip)
		return;
	}
	if ((pHeader->IdType == FDCAN_STANDARD_ID) && (pHeader->FDFormat == FDCAN_CLASSIC_CAN) && (pHeader->RxFrameType == FDCAN_DATA_FRAME) && (pHeader->DataLength <= 8))
	{
		// Apply the filter
		if ((pHeader->Identifier & filter_mask) != (filter_id & filter_mask)) {
			// Identifier does not match the filter
			return;
		}

		// Store the message in the buffer
		while (xSemaphoreTake(CANLOG_SEMAPHORE, 5 ) != pdTRUE){
			if (xStreamBufferBytesAvailable(CANRxDataStreamBufferHandle) > MAX_BUFFER_BYTES) return;
		}
		CAN_Message *message = &canMessageBuffer.messages[canMessageBuffer.head];
		message->identifier = pHeader->Identifier;
		message->payload_size = pHeader->DataLength;

		memcpy(message->data, data, pHeader->DataLength);

		// Update the buffer head and count
		canMessageBuffer.head = (canMessageBuffer.head + 1) % CAN_MESSAGE_BUFFER_SIZE;
		if (canMessageBuffer.count < CAN_MESSAGE_BUFFER_SIZE) {
			canMessageBuffer.count++;
		}
		xSemaphoreGive(CANLOG_SEMAPHORE);
	}
}


static void draw_header()
{
	uint8_t tmp[8];
	uint12toASCII(filter_id, tmp);
	tmp[3] = ':';
	uint12toASCII(filter_mask, &tmp[4]);
	tmp[7] = 0;

	//RAMN_SPI_DrawRectangle(0, 0, LCD_WIDTH, 16+6, SPI_COLOR_THEME.BACKGROUND);
	RAMN_SPI_DrawContour(0, 0, LCD_WIDTH, 16+6, CONTOUR_WIDTH, SPI_COLOR_THEME.LIGHT);
	RAMN_SPI_DrawStringColor2(5,5, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "RX DUMP");
	RAMN_SPI_DrawStringColor2(5+8*11,5, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, tmp);

	if(active)
	{
		RAMN_SPI_DrawStringColor2(5+16*11,5, SPI_COLOR_THEME.WHITE, SPI_COLOR_THEME.BACKGROUND, "  ON");
	}
	else
	{
		RAMN_SPI_DrawStringColor2(5+16*11,5, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "STOP");
	}

}

static void ScreenCANLog_Init() {

	if (CANLOG_SEMAPHORE == 0U) CANLOG_SEMAPHORE = xSemaphoreCreateMutexStatic(&CANLOG_SEMAPHORE_STRUCT);
	RAMN_ScreenUtils_PrepareScrollScreen();
	disp_index = 0;
	draw_header();
}

#define CANVAS_OFFSET SCREEN_HEADER_SIZE
static void ScreenCANLog_Update(uint32_t tick) {

	if (xStreamBufferBytesAvailable(CANRxDataStreamBufferHandle) > MAX_BUFFER_BYTES)
	{
		//Too many CAN messages available (would be immediately overwritten, skip)
		return;
	}
	if (spi_refresh_counter % 5 == 0)
	{
		while (xSemaphoreTake(CANLOG_SEMAPHORE, portMAX_DELAY ) != pdTRUE);

		if (active && (canMessageBuffer.count != 0)) {

			uint32_t start_index = (canMessageBuffer.head + CAN_MESSAGE_BUFFER_SIZE - canMessageBuffer.count) % CAN_MESSAGE_BUFFER_SIZE;
			for (uint8_t i = 0; i < canMessageBuffer.count; i++) {
				uint32_t index = (start_index + i) % CAN_MESSAGE_BUFFER_SIZE;
				CAN_Message *message = &canMessageBuffer.messages[index];
				uint8_t tmp[21];
				tmp[20] = 0;

				uint12toASCII(message->identifier, tmp);
				tmp[3] = ' ';
				for(uint8_t i=0;i<message->payload_size*2;i++)
				{
					uint4toASCII((message->data[i/2] >> (4*((i+1)%2)))&0xF,&tmp[4+i]);
				}
				tmp[4+message->payload_size*2] = 0;
				if (disp_index < CAN_MESSAGE_BUFFER_SIZE)
				{
						RAMN_SPI_DrawStringColor2(9, CANVAS_OFFSET+(16*(disp_index)), SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, tmp);
					if (8 - message->payload_size > 0)
					{
						RAMN_SPI_DrawRectangle(9+(4*11)+message->payload_size*22,CANVAS_OFFSET+(16*(disp_index)),22*(8 - message->payload_size),14,SPI_COLOR_THEME.BACKGROUND);
					}
				}
				else
				{
					//Must scroll and display at last line
					RAMN_SPI_ScrollUp(16);
					RAMN_SPI_DrawStringColor2(9, CANVAS_OFFSET+(16*(disp_index%SCREEN_BUFFER_MESSAGE_COUNT)), SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, tmp);
					if (8 - message->payload_size > 0)
					{
						RAMN_SPI_DrawRectangle(9+(4*11)+message->payload_size*22,CANVAS_OFFSET+(16*(disp_index%SCREEN_BUFFER_MESSAGE_COUNT)),22*(8 - message->payload_size),14,SPI_COLOR_THEME.BACKGROUND);
					}
				}

				disp_index += 1;
			}

			// Empty buffer
			canMessageBuffer.head = 0;
			canMessageBuffer.count = 0;
		}
		xSemaphoreGive(CANLOG_SEMAPHORE);
	}
}

static void ScreenCANLog_Deinit() {
	RAMN_SPI_SetScroll(SCREEN_HEADER_SIZE);
}

static void ScreenCANLog_UpdateInput(JoystickEventType event) {

	if (event == JOYSTICK_EVENT_DOWN_PRESSED)
	{
		if (filter_index == 0) filter_index = NUMBER_OF_FILTERS-1;
		else filter_index--;
		setCANfilterIndex(filter_index);
		draw_header();
	}
	else if (event == JOYSTICK_EVENT_UP_PRESSED)
	{
		filter_index = (filter_index + 1) % NUMBER_OF_FILTERS;
		setCANfilterIndex(filter_index);
		draw_header();
	}
	else if (event == JOYSTICK_EVENT_CENTER_PRESSED)
	{
		active = !active;
		draw_header();
	}

}

RAMNScreen ScreenCANLog = {
		.Init = ScreenCANLog_Init,
		.Update = ScreenCANLog_Update,
		.Deinit = ScreenCANLog_Deinit,
		.UpdateInput = ScreenCANLog_UpdateInput
};

#endif
