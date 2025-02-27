/*
 * ramn_screen.c
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

#include "ramn_screen_manager.h"

#ifdef ENABLE_SCREEN

// Private variables ---------------------

// Current screen
static RAMNScreen_t *currentScreen = NULL;

// Array of all possible screens
static RAMNScreen_t* screens[] = {
		&ScreenSaver,
		&ScreenCANMonitor,
		&ScreenCANLog,
		&ScreenStats,
#ifdef ENABLE_CHIP8
		&ScreenChip8,
#endif
#if defined(ENABLE_UDS) && !defined(HARDENING)
		&ScreenUDS
#endif
}; //TODO: move to flash (?)

// Private functions ---------------------

// Ends current screen and starts the one provided as argument.
void switchScreen(RAMNScreen_t* newScreen)
{
	if (currentScreen != NULL) {
		if (currentScreen->Deinit != NULL) currentScreen->Deinit();
	}
	currentScreen = newScreen;
	if (currentScreen != NULL) {
		if (currentScreen->Init != NULL) currentScreen->Init();
	}
}

// Finds and starts the next screen in the screens array.
static void moveScreen(int8_t direction)
{
	for (int8_t i = 0; i < SCREEN_COUNT; i++)
	{
		if (currentScreen == screens[i])
		{
			int8_t nextIndex = (i + direction + SCREEN_COUNT) % SCREEN_COUNT;
			switchScreen(screens[nextIndex]);
			break;
		}
	}
	RAMN_SCREENUTILS_LoopCounter = 0U; // Reset the loop counter.
}

// Called when user press joystick right.
static void changeScreenRight()
{
	moveScreen(1);
}

// Called when user press joystick left.
static void changeScreenLeft()
{
	moveScreen(-1);
}

// Public functions ---------------------

void RAMN_SCREENMANAGER_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask)
{
	RAMN_SCREENUTILS_Init(handler, pTask);
	switchScreen(DEFAULT_SCREEN);
}

void RAMN_SCREENMANAGER_Update(uint32_t tick)
{
	JoystickEventType joystickEvent;
	RAMN_Bool_t canProcessInput = True;

#ifdef ENABLE_CHIP8
	// Force to move to CHIP8 screen if a game was requested (e.g., by USB or by UDS).
	if (RAMN_CHIP8_IsGameActive() && currentScreen != &ScreenChip8)
	{
		switchScreen(&ScreenChip8);
	}
#endif

	// Force to move to the UDS screen if a UDS draw was requested.
#ifdef ENABLE_UDS
	if ((RAMN_SCREENUDS_RedrawNeeded != 0U) && (currentScreen != &ScreenUDS))
	{
		switchScreen(&ScreenUDS);
	}
#endif

	joystickEvent = RAMN_Joystick_Pop();

	while (joystickEvent != JOYSTICK_EVENT_NONE)
	{
		if (currentScreen != NULL) {
			if (currentScreen->UpdateInput != NULL) canProcessInput = currentScreen->UpdateInput(joystickEvent);
		}

		if (canProcessInput == True) // Controls not overridden by screen
		{
			if (joystickEvent == JOYSTICK_EVENT_RIGHT_PRESSED) changeScreenRight();
			else if (joystickEvent == JOYSTICK_EVENT_LEFT_PRESSED) changeScreenLeft();
		}

		joystickEvent = RAMN_Joystick_Pop(); // Get next event

	}

	if (RAMN_SCREENUTILS_RequestRedraw != False)
	{
		switchScreen(currentScreen);
		RAMN_SCREENUTILS_RequestRedraw = False;
	}

	// Update current screen.
	if (currentScreen != NULL) {
		if (currentScreen->Update != NULL) currentScreen->Update(tick);
	}

#ifdef DISPLAY_SLOW_WARNING
	// Display a message if the loop execution takes too much time.
	if (RAMN_SCREENUTILS_LoopCounter >= SLOW_WARNING_MIN_LOOP_COUNT && (xTaskGetTickCount() - tick) > SIM_LOOP_CLOCK_MS)
	{
		RAMN_SPI_DrawString(5, 5, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, RAMN_SCREENUTILS_COLORTHEME.LIGHT, "SLOW: ");
		RAMN_SPI_DrawUint32(5+(6*11), 5, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, RAMN_SCREENUTILS_COLORTHEME.LIGHT, xTaskGetTickCount() - tick);
	}
#endif

	RAMN_SCREENUTILS_LoopCounter += 1U;
}

void RAMN_SCREENMANAGER_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick)
{
	if (currentScreen != NULL) {
		if (currentScreen->ProcessRxCANMessage != NULL) currentScreen->ProcessRxCANMessage(pHeader, data, tick);
	}
}

#ifdef ENABLE_CHIP8

void RAMN_SCREENMANAGER_RequestGame(const uint8_t* game_to_load, uint16_t game_size)
{
	RAMN_SCREENCHIP8_RequestGame(game_to_load, game_size);

}

void RAMN_SCREENMANAGER_StartGameFromIndex(uint8_t index)
{
	RAMN_SCREENCHIP8_StartGameFromIndex(index);
}

#endif

#ifdef ENABLE_UDS

RAMN_Bool_t RAMN_SCREENMANAGER_IsUDSScreenUpdatePending()
{
	return RAMN_SCREENUDS_RedrawNeeded;
}

void RAMN_ScreenManager_RequestDrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* image)
{
	RAMN_SCREENUDS_RequestDrawImage(x, y, w, h, image);
}

#endif
#endif
