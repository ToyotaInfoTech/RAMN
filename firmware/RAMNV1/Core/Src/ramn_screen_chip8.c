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
	RAMN_SCREENUTILS_DrawBase();
	RAMN_CHIP8_SetColor(RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND);
	if (RAMN_CHIP8_IsGameActive()) RAMN_CHIP8_RedrawScreen();
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
			RAMN_SPI_DrawString(5,32+(i*16), RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, GAME_TITLES[i]);
		}
		else
		{
			RAMN_SPI_DrawString(5,32+(i*16), RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, RAMN_SCREENUTILS_COLORTHEME.LIGHT, GAME_TITLES[i]);
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
			RAMN_SPI_DrawString(5,160-16, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "Turn Key to quit.");
			RAMN_SPI_DrawString(5,160, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "Use BRAKE to control\rgame speed.");

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
		RAMN_SCREENUTILS_DrawBase();
		RAMN_SPI_DrawString(90,5, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "CHIP-8");

		drawGameTitles();
		menu_is_drawn = 1U;
	}

	if (RAMN_SCREENUTILS_LoopCounter % 5 == 0)
	{
		RAMN_SCREENUTILS_DrawSubconsoleUpdate();
	}
}

static void ScreenChip8_Deinit() {
}

static RAMN_Bool_t ScreenChip8_UpdateInput(JoystickEventType event) {
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
		return True;
	}
	else return False; // Game is ongoing, ask screen manager to not process inputs
}

RAMNScreen ScreenChip8 = {
		.Init = ScreenChip8_Init,
		.Update = ScreenChip8_Update,
		.Deinit = ScreenChip8_Deinit,
		.UpdateInput = ScreenChip8_UpdateInput,
		.ProcessRxCANMessage = 0U
};

#endif
