/*
 * ramn_screen_uds.c
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

#include "ramn_screen_uds.h"

#ifdef ENABLE_UDS
static uint8_t menu_drawn = 0U;
uint8_t uds_draw_need_refresh = 0U;

uint8_t uds_draw_x = 0;
uint8_t uds_draw_y = 0;
uint8_t uds_draw_w = 0;
uint8_t uds_draw_h = 0;
volatile uint8_t uds_draw_buffer[UDS_DRAW_BUFFER_SIZE];
#endif


void RAMN_ScreenUDS_RequestDrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* image)
{
#ifdef ENABLE_UDS
	if (uds_draw_buffer != 0U) osDelay(10);
	uds_draw_x = x;
	uds_draw_y = y;
	uds_draw_w = w;
	uds_draw_h = h;
	memcpy(uds_draw_buffer, image, w*h*2);
	uds_draw_need_refresh = 1U;
#endif
}

static void ScreenUDS_Init() {
	RAMN_ScreenUtils_DrawBase(current_theme);
	RAMN_SPI_DrawStringColor(75,5, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "UDS DRAW");
	RAMN_SPI_DrawStringColor(10,5+32, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "Use UDS Service to\ndraw an image here.");
	RAMN_SPI_DrawStringColor(10,5+80, SPI_COLOR_THEME.BACKGROUND,  SPI_COLOR_THEME.LIGHT, "ramn.readthedocs.io");
}

static void ScreenUDS_Update(uint32_t tick) {

#ifdef ENABLE_UDS

	if (uds_draw_need_refresh != 0U)
	{
		RAMN_SPI_DrawImage(uds_draw_x,uds_draw_y,uds_draw_w,uds_draw_h,uds_draw_buffer);
		uds_draw_need_refresh = 0U;
	}
#endif

}

static void ScreenUDS_Deinit() {
}

static void ScreenUDS_UpdateInput(JoystickEventType event) {
}

RAMNScreen ScreenUDS = {
		.Init = ScreenUDS_Init,
		.Update = ScreenUDS_Update,
		.Deinit = ScreenUDS_Deinit,
		.UpdateInput = ScreenUDS_UpdateInput
};
