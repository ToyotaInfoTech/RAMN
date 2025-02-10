/*
 * ramn_screen_chip8.c
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

#include "ramn_screen_chip8.h"

#if defined(ENABLE_CHIP8) && defined(ENABLE_SCREEN)

char* GAME_TITLES[] = {"Danmaku", "Cave Explorer", "Octopeg"};
#define NUMBER_OF_GAMES (sizeof(GAME_TITLES)/sizeof(char*))

static uint8_t gameIndex = 0U;
static uint8_t prevEngineKeyStatus = 0U;
static RAMN_Bool_t menuDrawn = False;

static void drawGameTitles()
{
	for (uint8_t i = 0; i < NUMBER_OF_GAMES; i++)
	{
		if (i != gameIndex) RAMN_SPI_DrawString(5,32+(i*16), RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, GAME_TITLES[i]);
		else RAMN_SPI_DrawString(5,32+(i*16), RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, RAMN_SCREENUTILS_COLORTHEME.LIGHT, GAME_TITLES[i]);
	}
}

static void SCREENCHIP8_Init()
{
	RAMN_SCREENUTILS_DrawBase();
	RAMN_CHIP8_SetColor(RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND);
	if (RAMN_CHIP8_IsGameActive()) RAMN_CHIP8_RedrawScreen();
	menuDrawn = False;
}

void RAMN_SCREENCHIP8_RequestGame(const uint8_t* game_to_load, uint16_t game_size)
{
	RAMN_CHIP8_StopGame();
	osDelay(100); //leave some time to quit the game
	RAMN_CHIP8_Init(game_to_load, game_size);
	RAMN_CHIP8_StartGame(xTaskGetTickCount());
	prevEngineKeyStatus = RAMN_DBC_Handle.control_enginekey&0xFF;
}

void RAMN_SCREENCHIP8_StartGameFromIndex(uint8_t index)
{
	switch(index){
	case 0x01:
		RAMN_SCREENCHIP8_RequestGame(DANMAKU, DANMAKU_SIZE);
		break;
	case 0x02:
		RAMN_SCREENCHIP8_RequestGame(CAVE_EXPLORER, CAVE_EXPLORER_SIZE);
		break;
	case 0x03:
		RAMN_SCREENCHIP8_RequestGame(OCTOPEG, OCTOPEG_SIZE);
		break;
	default:
		break;
	}
}

static void SCREENCHIP8_Update(uint32_t tick)
{
	if (RAMN_CHIP8_IsGameActive())
	{
		if (menuDrawn == True)
		{
			SCREENCHIP8_Init();
			menuDrawn = False;
			RAMN_SPI_DrawString(5,160-16, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "Turn Key to quit.");
			RAMN_SPI_DrawString(5,160, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "Use BRAKE to control\rgame speed.");
		}
		//Adjust game speed (number of instructions) based on current brake slider position
		uint16_t loop_max_count = 1+4*(100 - RAMN_DBC_Handle.control_brake*100 /(0xfff));
		for(uint16_t i = 0; i < loop_max_count; i++) {
			RAMN_CHIP8_Update(tick);
		}

		if ((RAMN_DBC_Handle.control_enginekey &0xFF) != prevEngineKeyStatus) RAMN_CHIP8_StopGame(1);
	}
	else if (menuDrawn == False)
	{
		RAMN_SCREENUTILS_DrawBase();
		RAMN_SPI_DrawString(90,5, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "CHIP-8");
		drawGameTitles();
		menuDrawn = True;
	}

	if (RAMN_SCREENUTILS_LoopCounter % 5U == 0U) RAMN_SCREENUTILS_DrawSubconsoleUpdate();
}

static RAMN_Bool_t SCREENCHIP8_UpdateInput(JoystickEventType event)
{
	if (!RAMN_CHIP8_IsGameActive())
	{
		if (event == JOYSTICK_EVENT_DOWN_PRESSED)
		{
			gameIndex = (gameIndex + 1) % NUMBER_OF_GAMES;
			drawGameTitles();
		}
		else if (event == JOYSTICK_EVENT_UP_PRESSED)
		{
			if (gameIndex == 0U) gameIndex = NUMBER_OF_GAMES - 1;
			else gameIndex = (gameIndex - 1);
			drawGameTitles();
		}
		else if (event == JOYSTICK_EVENT_CENTER_PRESSED)
		{
			RAMN_SCREENCHIP8_StartGameFromIndex(gameIndex+1);
		}
		return True;
	}
	else return False; // Game is ongoing, ask screen manager to not process inputs
}

RAMNScreen ScreenChip8 = {
		.Init = SCREENCHIP8_Init,
		.Update = SCREENCHIP8_Update,
		.Deinit = 0U,
		.UpdateInput = SCREENCHIP8_UpdateInput,
		.ProcessRxCANMessage = 0U
};

#endif
