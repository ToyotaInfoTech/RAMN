/*
 * ramn_screen_saver.h
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

#include "ramn_screen_saver.h"


char ascii_string[16] = {0};
char random_char_line[19] = {0};

static void ScreenSaver_Init() {
	RAMN_ScreenUtils_DrawBase(current_theme);
}

static void ScreenSaver_Update(uint32_t tick) {

	//random value for the "digital rain" effect on screen
	uint16_t random_colors[] = {SPI_COLOR_THEME.DARK, SPI_COLOR_THEME.DARK, SPI_COLOR_THEME.MEDIUM, SPI_COLOR_THEME.MEDIUM, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.WHITE};
	uint8_t random_X_line = RAMN_RNG_Pop8() % sizeof(random_char_line);
	uint8_t random_Y_line = RAMN_RNG_Pop8() % 12;
	uint8_t random_val = RAMN_RNG_Pop8();
	uint16_t color = random_colors[random_val % ((sizeof(random_colors)/sizeof(uint16_t)))];
	uint8_t random_char = (random_val % 75) + '0';

	RAMN_SPI_DrawCharColor(5+(random_X_line*12), 5+(random_Y_line*16), color, SPI_COLOR_THEME.BACKGROUND, random_char);

	//Code to display a message if problems happened happened
	if (spi_refresh_counter % 5 == 0)
	{
		if ((RAMN_FDCAN_Status.slcan_flags & (SLCAN_FLAG_RX_QUEUE_FULL | SLCAN_FLAG_TX_QUEUE_FULL | SLCAN_FLAG_DATA_OVERRUN)) != 0)
		{
			memcpy(ascii_string,"OVF",4);
			RAMN_SPI_DrawStringColor(5, 5+(0*16), SPI_COLOR_THEME.BACKGROUND, SPI_COLOR_THEME.LIGHT, ascii_string);
		}

		if (RAMN_FDCAN_Status.CANErrCnt > 0U)
		{
			memcpy(ascii_string,"ERR",4);
			RAMN_SPI_DrawStringColor(5, 5+(1*16), SPI_COLOR_THEME.BACKGROUND, SPI_COLOR_THEME.LIGHT, ascii_string);
		}

		if (RAMN_USB_Config.USBErrCnt > 0U)
		{
			memcpy(ascii_string,"USB",4);
			RAMN_SPI_DrawStringColor(5, 5+(2*16), SPI_COLOR_THEME.BACKGROUND, SPI_COLOR_THEME.LIGHT, ascii_string);
		}

		RAMN_ScreenUtils_DrawSubconsoleUpdate();
	}

}

static void ScreenSaver_Deinit() {

}



static void ScreenSaver_UpdateInput(JoystickEventType event) {

	// As an example, change color theme if center button is pressed
	if (event == JOYSTICK_EVENT_CENTER_PRESSED)
	{
		RAMN_ScreenUtils_UpdateTheme((current_theme%(NUMBER_OF_THEMES-1))+1);
	};

}

RAMNScreen ScreenSaver = {
    .Init = ScreenSaver_Init,
    .Update = ScreenSaver_Update,
    .Deinit = ScreenSaver_Deinit,
	.UpdateInput = ScreenSaver_UpdateInput
};
