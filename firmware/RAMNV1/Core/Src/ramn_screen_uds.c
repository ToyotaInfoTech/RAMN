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

#ifdef ENABLE_SCREEN

#ifdef ENABLE_UDS
uint8_t uds_draw_need_refresh = 0U;

uint8_t uds_draw_x = 0;
uint8_t uds_draw_y = 0;
uint8_t uds_draw_w = 0;
uint8_t uds_draw_h = 0;
__attribute__ ((section (".buffers"))) volatile uint8_t uds_draw_buffer[UDS_DRAW_BUFFER_SIZE];
#endif


void RAMN_ScreenUDS_RequestDrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* image)
{
#ifdef ENABLE_UDS
	if (uds_draw_buffer != 0U) osDelay(10);
	uds_draw_x = x;
	uds_draw_y = y;
	uds_draw_w = w;
	uds_draw_h = h;
	RAMN_memcpy(uds_draw_buffer, image, w*h*2);
	uds_draw_need_refresh = 1U;
#endif
}

static void ScreenUDS_Init() {
	RAMN_SCREENUTILS_DrawBase();
	RAMN_SPI_DrawString(75,5, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "UDS DRAW");
	RAMN_SPI_DrawString(10,5+32, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "Use UDS Service to\ndraw an image here.");
	RAMN_SPI_DrawString(10,5+80, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND,  RAMN_SCREENUTILS_COLORTHEME.LIGHT, "ramn.readthedocs.io");
}

static void ScreenUDS_Update(uint32_t tick) {

#ifdef ENABLE_UDS

	if (uds_draw_need_refresh != 0U)
	{
		RAMN_SPI_DrawImage(uds_draw_x,uds_draw_y,uds_draw_w,uds_draw_h,uds_draw_buffer);
		uds_draw_need_refresh = 0U;
	}
#endif

	if (RAMN_SCREENUTILS_LoopCounter % 5 == 0)
	{
		RAMN_SCREENUTILS_DrawSubconsoleUpdate();
	}
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

#endif
