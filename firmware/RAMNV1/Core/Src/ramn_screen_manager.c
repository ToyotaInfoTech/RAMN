/*
 * ramn_screen.c
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

#include <ramn_screen_manager.h>

#ifdef ENABLE_SCREEN

static RAMNScreen *currentScreen = NULL;

#define DEFAULT_SCREEN &ScreenSaver


static void changeScreenRight()
{
	if (currentScreen == &ScreenSaver)
	{
		RAMN_ScreenManager_SwitchScreen(&ScreenCANMonitor);
	}
	else if (currentScreen == &ScreenCANMonitor)
	{
		RAMN_ScreenManager_SwitchScreen(&ScreenChip8);
	}
	else if (currentScreen == &ScreenChip8)
	{
		RAMN_ScreenManager_SwitchScreen(&ScreenUDS);
	}
	else if (currentScreen == &ScreenUDS)
	{
		RAMN_ScreenManager_SwitchScreen(&ScreenSaver);
	}
}

static void changeScreenLeft()
{
	if (currentScreen == &ScreenSaver)
	{
		RAMN_ScreenManager_SwitchScreen(&ScreenUDS);
	}
	else if (currentScreen == &ScreenCANMonitor)
	{
		RAMN_ScreenManager_SwitchScreen(&ScreenSaver);
	}
	else if (currentScreen == &ScreenChip8)
	{
		RAMN_ScreenManager_SwitchScreen(&ScreenCANMonitor);
	}
	else if (currentScreen == &ScreenUDS)
	{
		RAMN_ScreenManager_SwitchScreen(&ScreenChip8);
	}
}


void RAMN_ScreenManager_SwitchScreen(RAMNScreen* newScreen)
{
	if (currentScreen != NULL) {
		if (currentScreen->Deinit != 0U) currentScreen->Deinit();
	}
	currentScreen = newScreen;
	if (currentScreen != NULL) {
		if (currentScreen->Init != 0U) currentScreen->Init();
	}
}

void RAMN_ScreenManager_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask)
{
	RAMN_ScreenUtils_Init(handler, pTask);

	RAMN_ScreenManager_SwitchScreen(DEFAULT_SCREEN);
}

void RAMN_ScreenManager_RequestGame(const uint8_t* game_to_load, uint16_t game_size)
{
	RAMN_ScreenChip8_RequestGame(game_to_load, game_size);

}

void RAMN_ScreenManager_StartGameFromIndex(uint8_t index)
{
	RAMN_ScreenChip8_StartGameFromIndex(index);
}


void RAMN_SCREEN_Update(uint32_t tick)
{


	if (RAMN_CHIP8_IsGameActive() && currentScreen != &ScreenChip8)
	{
		RAMN_ScreenManager_SwitchScreen(&ScreenChip8);
	}

	if ((uds_draw_need_refresh != 0U) && (currentScreen != &ScreenUDS))
	{
		RAMN_ScreenManager_SwitchScreen(&ScreenUDS);
	}

	//No game active, monitor inputs for screen changes
	JoystickEventType joystick_action = RAMN_Joystick_Pop();

	while (joystick_action != JOYSTICK_EVENT_NONE)

	{
		if (joystick_action >  JOYSTICK_EVENT_RIGHT_RELEASED)
		{
			//Do not pass events LEFT/RIGHT, used to switch between screens.
			if (currentScreen != NULL) {
				if (currentScreen->UpdateInput != 0U) currentScreen->UpdateInput(joystick_action);
			}
		}
		else if (!RAMN_CHIP8_IsGameActive())
		{
			//Left/Right press while game is unactive, change screen
			if (joystick_action == JOYSTICK_EVENT_RIGHT_PRESSED)
			{
				changeScreenRight();
			}
			else if (joystick_action == JOYSTICK_EVENT_LEFT_PRESSED)
			{
				changeScreenLeft();
			}
		}
		joystick_action = RAMN_Joystick_Pop(); //get next

	}

	if (theme_change_requested != 0U)
	{
		RAMN_ScreenManager_SwitchScreen(currentScreen);
		//		if (RAMN_CHIP8_IsGameActive()) RAMN_Chip8_RedrawScreen();
		theme_change_requested = 0U;
	}

	if (currentScreen != NULL) {
		if (currentScreen->Update != 0)
		{
			currentScreen->Update(tick);
		}
	}

	spi_refresh_counter += 1;

	//Example to scroll screen
	//RAMN_SPI_SetScroll(SCREEN_HEADER_SIZE + ((tick/10)%(SCROLL_WINDOW_HEIGHT-SCREEN_HEADER_SIZE)));


	//Code to display a message if a loop execution takes too much time
	//	if (spi_refresh_counter > 0 && (xTaskGetTickCount() - tick) > SIM_LOOP_CLOCK_MS)
	//	{
	//		//lastHornCountDisplayed = RAMN_DBC_Handle.horn_count;
	//		uint16toASCII(RAMN_DBC_Handle.horn_count&0xFFFF,(uint8_t*)cntStr);
	//		memcpy(cntStr,"SLW",4);
	//		RAMN_SPI_DrawStringColor(5, 5+(1*16), SPI_COLOR_THEME.BACKGROUND, SPI_COLOR_THEME.LIGHT, cntStr);
	//	}

}

uint8_t RAMN_ScreenManager_IsUDSScreenUpdatePending()
{
#ifdef ENABLE_UDS
	return uds_draw_need_refresh;
#else
	return 0U;
#endif
}


void RAMN_ScreenManager_RequestDrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* image)
{
	RAMN_ScreenUDS_RequestDrawImage(x, y, w, h, image);
}

void RAMN_ScreenManager_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick)
{
	if (currentScreen == &ScreenCANMonitor)
	{
		RAMN_ScreenCANMonitor_ProcessRxCANMessage(pHeader, data, tick);
	}
}




#endif
