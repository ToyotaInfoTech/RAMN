/*
 * ramn_screen_saver.h
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

#include "ramn_screen_saver.h"

#ifdef ENABLE_SCREEN

#define NUMBER_OF_LINES 19

static RAMN_Bool_t menuDrawn = False;

static void SCREENSAVER_Init()
{
	RAMN_SCREENUTILS_DrawBase();
}

static void SCREENSAVER_Update(uint32_t tick)
{
	if (menuDrawn == False)
	{
		menuDrawn = True;
		RAMN_SPI_DrawString(5+22,5+64, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, " Use ECU C Shift \rto control screen");
	}
	if (tick > 5000U) // Don't erase display for the first 5 seconds
	{
		// Random value for the "digital rain" effect on screen
		uint16_t 	random_colors[] = {RAMN_SCREENUTILS_COLORTHEME.DARK, RAMN_SCREENUTILS_COLORTHEME.DARK, RAMN_SCREENUTILS_COLORTHEME.MEDIUM, RAMN_SCREENUTILS_COLORTHEME.MEDIUM, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.WHITE};
		uint8_t 	random_X_line = RAMN_RNG_Pop8() % NUMBER_OF_LINES;
		uint8_t 	random_Y_line = RAMN_RNG_Pop8() % 12;
		uint8_t 	random_val = RAMN_RNG_Pop8();
		uint16_t 	color = random_colors[random_val % ((sizeof(random_colors)/sizeof(uint16_t)))];
		uint8_t 	random_char = (random_val % 75) + '0';

		RAMN_SPI_DrawChar(5+(random_X_line*12), 5+(random_Y_line*16), color, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, random_char);
	}

	if (RAMN_SCREENUTILS_LoopCounter % 5U == 0U) RAMN_SCREENUTILS_DrawSubconsoleUpdate();

}

static RAMN_Bool_t SCREENSAVER_UpdateInput(JoystickEventType event)
{
	// Change color theme if center button is pressed
	if (event == JOYSTICK_EVENT_CENTER_PRESSED) RAMN_SCREENUTILS_NextTheme();
	return True;

}

RAMNScreen ScreenSaver =
{
		.Init = SCREENSAVER_Init,
		.Update = SCREENSAVER_Update,
		.Deinit = 0U,
		.UpdateInput = SCREENSAVER_UpdateInput,
		.ProcessRxCANMessage = 0U
};

#endif
