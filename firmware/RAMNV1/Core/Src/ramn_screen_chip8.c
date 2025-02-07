/*
 * ramn_screen_chip8.c
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

#include "ramn_screen_chip8.h"

#ifdef ENABLE_SCREEN


static uint8_t menu_is_drawn = 0U;

static void ScreenChip8_Init() {
	RAMN_ScreenUtils_DrawBase(current_theme);
	menu_is_drawn = 0U;
}

char* GAME_TITLES[] = {"Danmaku", "Cave Explorer", "Octopeg"};
#define NUMBER_OF_GAMES (sizeof(GAME_TITLES)/sizeof(char*))
uint8_t selected_game_index = 0U;
uint8_t engine_key_status_when_started = 0U;


static void drawGameTitles()
{
	for (uint8_t i = 0; i < NUMBER_OF_GAMES; i++)
	{
		if (i != selected_game_index)
		{
			RAMN_SPI_DrawString(5,32+(i*16), SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, GAME_TITLES[i]);
		}
		else
		{
			RAMN_SPI_DrawString(5,32+(i*16), SPI_COLOR_THEME.BACKGROUND, SPI_COLOR_THEME.LIGHT, GAME_TITLES[i]);
		}
	}
}

void RAMN_ScreenChip8_RequestGame(const uint8_t* game_to_load, uint16_t game_size)
{
	RAMN_CHIP8_StopGame();
	osDelay(100); //leave some time to quit the game (TODO optimize this)
	RAMN_CHIP8_Init(game_to_load, game_size);
	RAMN_CHIP8_StartGame(xTaskGetTickCount());
	engine_key_status_when_started = RAMN_DBC_Handle.control_enginekey &0xFF;
}

void RAMN_ScreenChip8_StartGameFromIndex(uint8_t index)
{
	switch(index){
	case 0x01:
		RAMN_ScreenChip8_RequestGame(DANMAKU, DANMAKU_SIZE);
		break;
	case 0x02:
		RAMN_ScreenChip8_RequestGame(CAVE_EXPLORER, CAVE_EXPLORER_SIZE);
		break;
	case 0x03:
		RAMN_ScreenChip8_RequestGame(OCTOPEG, OCTOPEG_SIZE);
		break;
	default:
		break;
	}
}

static void ScreenChip8_Update(uint32_t tick) {
	if (RAMN_CHIP8_IsGameActive())
	{
		if (menu_is_drawn != 0U)
		{
			ScreenChip8_Init();
			menu_is_drawn = 0U;
			RAMN_SPI_DrawString(5,160-16, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "Turn Key to quit.");
			RAMN_SPI_DrawString(5,160, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "Use BRAKE to control\rgame speed.");

		}
		//Adjust game speed (number of instructions) based on current brake slider position
		uint16_t loop_max_count = 1+4*(100 - RAMN_DBC_Handle.control_brake*100 /(0xfff));
		for(uint16_t i = 0; i < loop_max_count; i++) {
			RAMN_CHIP8_Update(tick);
		}

		if ((RAMN_DBC_Handle.control_enginekey &0xFF) != engine_key_status_when_started)
		{
			RAMN_CHIP8_StopGame(1);
		}

	}
	else if (menu_is_drawn == 0U)
	{
		RAMN_ScreenUtils_DrawBase(current_theme);
		RAMN_SPI_DrawString(90,5, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "CHIP-8");

		drawGameTitles();
		menu_is_drawn = 1U;
	}

	if (spi_refresh_counter % 5 == 0)
	{
		RAMN_ScreenUtils_DrawSubconsoleUpdate();
	}
}

static void ScreenChip8_Deinit() {
}

static void ScreenChip8_UpdateInput(JoystickEventType event) {
	if (!RAMN_CHIP8_IsGameActive())
	{
	if (event == JOYSTICK_EVENT_DOWN_PRESSED)
	{
		selected_game_index = (selected_game_index + 1) % NUMBER_OF_GAMES;
		drawGameTitles();
	}
	else if (event == JOYSTICK_EVENT_UP_PRESSED)
	{
		if (selected_game_index == 0U) selected_game_index = NUMBER_OF_GAMES - 1;
		else selected_game_index = (selected_game_index - 1);
		drawGameTitles();
	}
	else if (event == JOYSTICK_EVENT_CENTER_PRESSED)
	{
		RAMN_ScreenChip8_StartGameFromIndex(selected_game_index+1);
	}
	}
}

RAMNScreen ScreenChip8 = {
		.Init = ScreenChip8_Init,
		.Update = ScreenChip8_Update,
		.Deinit = ScreenChip8_Deinit,
		.UpdateInput = ScreenChip8_UpdateInput
};

#endif
