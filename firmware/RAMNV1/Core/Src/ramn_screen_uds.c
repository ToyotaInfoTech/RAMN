/*
 * ramn_screen_uds.c
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

#include "ramn_screen_uds.h"

#if defined(ENABLE_SCREEN) && defined(ENABLE_UDS)

RAMN_Bool_t RAMN_SCREENUDS_RedrawNeeded = False;

static uint16_t drawX = 0;
static uint16_t drawY = 0;
static uint16_t drawW = 0;
static uint16_t drawH = 0;
__attribute__ ((section (".buffers"))) static uint8_t drawBuf[UDS_DRAW_BUFFER_SIZE]; //TODO: move this do dynamic memory; add if (drawBuf != 0U) osDelay(10); make it volatile?


void RAMN_SCREENUDS_RequestDrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* image)
{
	drawX = x;
	drawY = y;
	drawW = w;
	drawH = h;
	RAMN_memcpy(drawBuf, image, w*h*2);
	RAMN_SCREENUDS_RedrawNeeded = True;
}

static void SCREENUDS_Init()
{
	RAMN_SCREENUTILS_DrawBase();
	RAMN_SPI_DrawString(75,5, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "UDS DRAW");
	RAMN_SPI_DrawString(10,5+32, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "Use UDS Service to\ndraw an image here.");
	RAMN_SPI_DrawString(10,5+80, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND,  RAMN_SCREENUTILS_COLORTHEME.LIGHT, "ramn.readthedocs.io");
}

static void SCREENUDS_Update(uint32_t tick)
{
	if (RAMN_SCREENUDS_RedrawNeeded != False)
	{
		RAMN_SPI_DrawImage(drawX, drawY, drawW, drawH, drawBuf);
		RAMN_SCREENUDS_RedrawNeeded = False;
	}
	if (RAMN_SCREENUTILS_LoopCounter % 5U == 0U) RAMN_SCREENUTILS_DrawSubconsoleUpdate();
}

RAMNScreen_t ScreenUDS =
{
		.Init = SCREENUDS_Init,
		.Update = SCREENUDS_Update,
		.Deinit = 0U,
		.UpdateInput = 0U,
		.ProcessRxCANMessage = 0U
};

#endif

