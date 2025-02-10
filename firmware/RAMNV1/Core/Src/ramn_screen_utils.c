/*
 * ramn_screen_utils.c
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

#include "ramn_screen_utils.h"

#ifdef ENABLE_SCREEN

// Exported variables

uint32_t 				RAMN_SCREENUTILS_LoopCounter = 0U;
volatile RAMN_Bool_t 	RAMN_SCREENUTILS_RequestRedraw = 0U;
volatile ColorTheme_t 	RAMN_SCREENUTILS_COLORTHEME;

// Private variables

// Currently selected theme
static uint8_t  themeIndex = 1U;

// Previously drawn sensor values
static uint16_t prevSteer;
static uint16_t prevAccel;
static uint16_t prevBrake;
static uint8_t  prevSidebrake;
static uint8_t  prevEnginekey;
static uint8_t  prevLamp;
static uint8_t  prevShift;

// Private functions

// Function to convert a uint16 to an ASCII string with a % at the end
static void uint16toBCDPercent(uint16_t val, char* dst)
{
	if (dst != 0U)
	{
		dst[0] = (val / 100) % 10 + '0';
		if (dst[0] == '0') dst[0] = ' ';
		dst[1] = (val / 10) % 10 + '0';
		if (dst[0] == ' ' && dst[1] == '0') dst[1] = ' ';
		dst[2] = (val % 10) + '0';
		dst[3] = '%';
		dst[4] = 0;
	}
}

// Function to convert a SIGNED int16 to an ASCII string with a L (-) or R (+) at the beginning and % at the end
static void uint16toBCDSteering(int16_t val, char* dst)
{
	if (dst != 0U)
	{
		if (val > 0)
		{
			dst[0] = 'R';
		}
		else
		{
			dst[0] = 'L';
			val = -val;
		}
		dst[1] = (val / 100) % 10 + '0';
		if (dst[1] == '0') dst[1] = ' ';
		dst[2] = (val / 10) % 10 + '0';
		if (dst[1] == ' ' && dst[2] == '0') dst[2] = ' ';
		dst[3] = (val % 10) + '0';
		dst[4] = '%';
		dst[5] = 0;
	}
}

// Configures the color theme structure
static void configureColorTheme()
{
	RAMN_SCREENUTILS_COLORTHEME.BACKGROUND = 0x0000;
	RAMN_SCREENUTILS_COLORTHEME.WHITE = 0xFFFF;
	switch(themeIndex)
	{
	case 0:
	case 1:
		RAMN_SCREENUTILS_COLORTHEME.DARK = 0x01e0;
		RAMN_SCREENUTILS_COLORTHEME.MEDIUM = 0x0462;
		RAMN_SCREENUTILS_COLORTHEME.LIGHT = 0x07e8;
		break;
	case 2:
		RAMN_SCREENUTILS_COLORTHEME.DARK = 0x210a;
		RAMN_SCREENUTILS_COLORTHEME.MEDIUM = 0x31b2;
		RAMN_SCREENUTILS_COLORTHEME.LIGHT = 0x52df;
		break;
	case 3:
		RAMN_SCREENUTILS_COLORTHEME.DARK = 0x7145;
		RAMN_SCREENUTILS_COLORTHEME.MEDIUM = 0xb1c7;
		RAMN_SCREENUTILS_COLORTHEME.LIGHT = 0xf9c7;
		break;
	case 4:
		RAMN_SCREENUTILS_COLORTHEME.DARK = 0x902a;
		RAMN_SCREENUTILS_COLORTHEME.MEDIUM = 0xe02f;
		RAMN_SCREENUTILS_COLORTHEME.LIGHT = 0xf8b2;
		break;
	case 5:
		RAMN_SCREENUTILS_COLORTHEME.WHITE = 0x0000;
		RAMN_SCREENUTILS_COLORTHEME.BACKGROUND = 0xFFFF;
		RAMN_SCREENUTILS_COLORTHEME.DARK = 0x9c92;
		RAMN_SCREENUTILS_COLORTHEME.MEDIUM = 0x6b4d;
		RAMN_SCREENUTILS_COLORTHEME.LIGHT = 0x4208;
		break;
	case 7:
		RAMN_SCREENUTILS_COLORTHEME.WHITE = 0x0000;
		RAMN_SCREENUTILS_COLORTHEME.BACKGROUND = 0xFFFF;
	case 6:
		RAMN_SCREENUTILS_COLORTHEME.DARK = 0x07e8;
		RAMN_SCREENUTILS_COLORTHEME.MEDIUM = 0x52df;
		RAMN_SCREENUTILS_COLORTHEME.LIGHT = 0xf9c7;
		break;
	default:
		RAMN_SCREENUTILS_COLORTHEME.DARK = 0x01e0;
		RAMN_SCREENUTILS_COLORTHEME.MEDIUM = 0x0462;
		RAMN_SCREENUTILS_COLORTHEME.LIGHT = 0x07e8;
		break;
	}
}

// Exported functions

void RAMN_SCREENUTILS_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask)
{
	RAMN_SPI_Init(handler, pTask);
	RAMN_SPI_InitScreen();

	themeIndex = RAMN_RNG_Pop8(); // Select a random theme at startup
	if (themeIndex > 127)
		themeIndex = 1; // Make theme 1 more likely, and don't select theme 5 to 7
	else  themeIndex %= 5;

	configureColorTheme();
	RAMN_SCREENUTILS_DrawBase();
}


void RAMN_SCREENUTILS_UpdateTheme(uint8_t theme)
{
	if (theme != themeIndex)
	{
		themeIndex = theme;
		configureColorTheme();
		RAMN_SCREENUTILS_RequestRedraw = True;
	}
}

void RAMN_SCREENUTILS_NextTheme()
{
	RAMN_SCREENUTILS_UpdateTheme((themeIndex%(NUMBER_OF_THEMES-1))+1);
}

void RAMN_SCREENUTILS_PrepareScrollScreen()
{
	RAMN_SPI_SetScroll(SCREEN_HEADER_SIZE);
	RAMN_SPI_DrawRectangle(0,0,LCD_WIDTH,SCROLL_WINDOW_HEIGHT,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND);
}

void RAMN_SCREENUTILS_DrawBase()
{
	RAMN_SPI_DrawRectangle(0,0,LCD_WIDTH,LCD_HEIGHT-32,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND);
	RAMN_SPI_DrawContour(0, 0, LCD_WIDTH, LCD_HEIGHT-41, CONTOUR_WIDTH, RAMN_SCREENUTILS_COLORTHEME.LIGHT);
	RAMN_SCREENUTILS_DrawSubconsoleStatic();
}

void RAMN_SCREENUTILS_DrawSubconsoleStatic()
{
	RAMN_SPI_DrawRectangle(0, LCD_HEIGHT-36, LCD_WIDTH, 36, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND);
	RAMN_SPI_DrawContour(0, LCD_HEIGHT-36, LCD_WIDTH, LCD_HEIGHT, 2, RAMN_SCREENUTILS_COLORTHEME.WHITE);
	RAMN_SPI_DrawString(2,CONTROL_WINDOW_Y,RAMN_SCREENUTILS_COLORTHEME.LIGHT,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND,"STEER BRAK ACCL");

	// Set values that will force redraw under normal conditions.
	prevSteer = 0xFFFF;
	prevAccel = 0xFFFF;
	prevBrake = 0xFFFF;
	prevSidebrake = 0xFF;
	prevEnginekey = 0xFF;
	prevLamp = 0xFF;
	prevShift = 0xFF;
}

void RAMN_SCREENUTILS_DrawSubconsoleUpdate()
{
	char cntStr[6] = {0}; // temporary variable to convert sensor values to drawable ASCII

	if (RAMN_DBC_Handle.control_steer != prevSteer)
	{
		prevSteer = RAMN_DBC_Handle.control_steer;
		uint16toBCDSteering((((int16_t)(RAMN_DBC_Handle.control_steer) - 0x800)*100)/0x7ff, cntStr);
		RAMN_SPI_RefreshString(2,CONTROL_WINDOW_Y+16,RAMN_SCREENUTILS_COLORTHEME.WHITE,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND,cntStr);
	}

	if (RAMN_DBC_Handle.control_brake != prevBrake)
	{
		prevBrake = RAMN_DBC_Handle.control_brake;
		uint16toBCDPercent((RAMN_DBC_Handle.control_brake*100 / (0xfff)),cntStr);
		RAMN_SPI_RefreshString(66,CONTROL_WINDOW_Y+16,RAMN_SCREENUTILS_COLORTHEME.WHITE,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND,cntStr);
	}

	if (RAMN_DBC_Handle.control_accel != prevAccel)
	{
		prevAccel = RAMN_DBC_Handle.control_accel;
		uint16toBCDPercent(RAMN_DBC_Handle.control_accel*100 / (0xfff),cntStr);
		RAMN_SPI_RefreshString(122,CONTROL_WINDOW_Y+16,RAMN_SCREENUTILS_COLORTHEME.WHITE,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND,cntStr);
	}

	if ((RAMN_DBC_Handle.joystick) != prevShift)
	{
		prevShift = (RAMN_DBC_Handle.joystick)&0xFF;
		switch(RAMN_DBC_Handle.joystick)
		{
		case RAMN_SHIFT_UP:
			strcpy(cntStr, "UP");
			break;
		case RAMN_SHIFT_DOWN:
			strcpy(cntStr, "DW");
			break;
		case RAMN_SHIFT_RIGHT:
			strcpy(cntStr, "RT");
			break;
		case RAMN_SHIFT_LEFT:
			strcpy(cntStr, "LT");
			break;
		case RAMN_SHIFT_PUSH:
			strcpy(cntStr, "MD");
			break;
		default:
			strcpy(cntStr, "  ");
			break;
		}
		RAMN_SPI_DrawString(170,CONTROL_WINDOW_Y+16,RAMN_SCREENUTILS_COLORTHEME.WHITE,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND,cntStr);
	}
	if (RAMN_DBC_Handle.control_sidebrake != prevSidebrake)
	{
		prevSidebrake = RAMN_DBC_Handle.control_sidebrake;
		if (RAMN_DBC_Handle.control_sidebrake != RAMN_SIDEBRAKE_DOWN) strcpy(cntStr, "SB");
		else strcpy(cntStr, "  ");
		RAMN_SPI_DrawString(210,CONTROL_WINDOW_Y,RAMN_SCREENUTILS_COLORTHEME.WHITE,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND,cntStr);
	}

	if ((RAMN_DBC_Handle.command_lights&0xFF) != prevLamp)
	{
		prevLamp = (RAMN_DBC_Handle.command_lights&0xFF);
		switch((RAMN_DBC_Handle.command_lights&0xFF))
		{
		case RAMN_LIGHTSWITCH_POS2:
			strcpy(cntStr, "CL");
			break;
		case RAMN_LIGHTSWITCH_POS3:
			strcpy(cntStr, "LB");
			break;
		case RAMN_LIGHTSWITCH_POS4:
			strcpy(cntStr, "HB");
			break;
		default:
			strcpy(cntStr, "  ");
			break;
		}
		RAMN_SPI_DrawString(180,CONTROL_WINDOW_Y,RAMN_SCREENUTILS_COLORTHEME.WHITE,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND,cntStr);
	}

	if ((RAMN_DBC_Handle.control_enginekey &0xFF) != prevEnginekey)
	{
		prevEnginekey = (RAMN_DBC_Handle.control_enginekey &0xFF);
		switch((RAMN_DBC_Handle.control_enginekey &0xFF))
		{
		case RAMN_ENGINEKEY_LEFT:
			strcpy(cntStr, "OFF");
			break;
		case RAMN_ENGINEKEY_MIDDLE:
			strcpy(cntStr, "ACC");
			break;
		case RAMN_ENGINEKEY_RIGHT:
			strcpy(cntStr, "IGN");
			break;
		default:
			strcpy(cntStr, "???");
			break;
		}
		RAMN_SPI_DrawString(200,CONTROL_WINDOW_Y+16,RAMN_SCREENUTILS_COLORTHEME.WHITE,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND,cntStr);
	}
}

#endif
