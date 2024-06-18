/*
 * ramn_screen_utils.c
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

#include <ramn_screen_utils.h>

#ifdef ENABLE_SCREEN

uint32_t spi_refresh_counter = 0U;

char prev_steer_ascii[6] = {0};
char prev_accel_ascii[5] = {0};
char prev_brake_ascii[5] = {0};
uint16_t prev_steer = 0xFFFF;
uint16_t prev_accel = 0xFFFF;
uint16_t prev_brake = 0xFFFF;
uint8_t prev_sidebrake = 0xFF;
uint8_t prev_enginekey = 0xFF;
uint8_t prev_lamp = 0xFF;
uint8_t prev_shift = 0xFF;

volatile ColorTheme_t SPI_COLOR_THEME;
uint16_t SPI_SUBCONSOLE_BACKGROUNDCOLOR;
uint16_t SPI_SUBCONSOLE_COLORCONTOUR;
uint16_t SPI_SUBCONSOLE_COLORSTATIC;
uint16_t SPI_SUBCONSOLE_COLORDYNAMIC;

volatile uint8_t current_theme = 1;
volatile uint8_t theme_change_requested = 0U;


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

void RAMN_ScreenUtils_DrawSubconsoleStatic()
{
	SPI_SUBCONSOLE_BACKGROUNDCOLOR = SPI_COLOR_THEME.BACKGROUND;
	SPI_SUBCONSOLE_COLORCONTOUR = SPI_COLOR_THEME.WHITE;
	SPI_SUBCONSOLE_COLORDYNAMIC = SPI_COLOR_THEME.WHITE;
	SPI_SUBCONSOLE_COLORSTATIC = SPI_COLOR_THEME.LIGHT;

	RAMN_SPI_DrawRectangle(0, LCD_HEIGHT-36, LCD_WIDTH, 36, SPI_SUBCONSOLE_BACKGROUNDCOLOR);
	RAMN_SPI_DrawContour(0, LCD_HEIGHT-36, LCD_WIDTH, LCD_HEIGHT, 2, SPI_SUBCONSOLE_COLORCONTOUR);
	RAMN_SPI_DrawStringColor(2,CONTROL_WINDOW_Y,SPI_SUBCONSOLE_COLORSTATIC,SPI_SUBCONSOLE_BACKGROUNDCOLOR,"STEER BRAK ACCL");

	//Reset strings
	memset(prev_steer_ascii,0,6);
	memset(prev_accel_ascii,0,5);
	memset(prev_brake_ascii,0,5);

	prev_steer = 0xFFFF;
	prev_accel = 0xFFFF;
	prev_brake = 0xFFFF;
	prev_sidebrake = 0xFF;
	prev_enginekey = 0xFF;
	prev_lamp = 0xFF;
	prev_shift = 0xFF;
}

void RAMN_ScreenUtils_DrawSubconsoleUpdate()
{
	char cntStr[6] = {0};

	if (RAMN_DBC_Handle.control_steer != prev_steer)
	{
		prev_steer = RAMN_DBC_Handle.control_steer;
		uint16toBCDSteering((((int16_t)(RAMN_DBC_Handle.control_steer) - 0x800)*100)/0x7ff, cntStr);
		RAMN_SPI_RefreshStringColor(2,CONTROL_WINDOW_Y+16,SPI_SUBCONSOLE_COLORDYNAMIC,SPI_SUBCONSOLE_BACKGROUNDCOLOR,cntStr, prev_steer_ascii);
	}

	if (RAMN_DBC_Handle.control_brake != prev_brake)
	{
		prev_brake = RAMN_DBC_Handle.control_brake;
		uint16toBCDPercent((RAMN_DBC_Handle.control_brake*100 / (0xfff)),cntStr);
		RAMN_SPI_RefreshStringColor(66,CONTROL_WINDOW_Y+16,SPI_SUBCONSOLE_COLORDYNAMIC,SPI_SUBCONSOLE_BACKGROUNDCOLOR,cntStr, prev_brake_ascii);
	}

	if (RAMN_DBC_Handle.control_accel != prev_accel)
	{
		prev_accel = RAMN_DBC_Handle.control_accel;
		uint16toBCDPercent(RAMN_DBC_Handle.control_accel*100 / (0xfff),cntStr);
		RAMN_SPI_RefreshStringColor(122,CONTROL_WINDOW_Y+16,SPI_SUBCONSOLE_COLORDYNAMIC,SPI_SUBCONSOLE_BACKGROUNDCOLOR,cntStr, prev_accel_ascii);
	}

	if ((RAMN_DBC_Handle.control_shift >> 8) != prev_shift)
	{
		prev_shift = (RAMN_DBC_Handle.control_shift >> 8)&0xFF;
		switch((RAMN_DBC_Handle.control_shift >> 8))
		{
		case 0x02:
			//up
			strcpy(cntStr, "UP");
			break;
		case 0x03:
			//down
			strcpy(cntStr, "DW");
			break;
		case 0x04:
			//right
			strcpy(cntStr, "RT");
			break;
		case 0x05:
			//left
			strcpy(cntStr, "LT");
			break;
		case 0x06:
			//center
			strcpy(cntStr, "MD");
			break;
		default:
			strcpy(cntStr, "  ");
			break;
		}
		RAMN_SPI_DrawStringColor(170,CONTROL_WINDOW_Y+16,SPI_SUBCONSOLE_COLORDYNAMIC,SPI_SUBCONSOLE_BACKGROUNDCOLOR,cntStr);
	}
	if (RAMN_DBC_Handle.control_sidebrake != prev_sidebrake)
	{
		prev_sidebrake = RAMN_DBC_Handle.control_sidebrake;
		if (RAMN_DBC_Handle.control_sidebrake != 0) strcpy(cntStr, "SB");
		else strcpy(cntStr, "  ");
		RAMN_SPI_DrawStringColor(210,CONTROL_WINDOW_Y,SPI_SUBCONSOLE_COLORDYNAMIC,SPI_SUBCONSOLE_BACKGROUNDCOLOR,cntStr);
	}

	if ((RAMN_DBC_Handle.command_lights&0xFF) != prev_lamp)
	{
		prev_lamp = (RAMN_DBC_Handle.command_lights&0xFF);
		switch((RAMN_DBC_Handle.command_lights&0xFF))
		{
		case 0x02:
			//down
			strcpy(cntStr, "CL");
			break;
		case 0x03:
			//right
			strcpy(cntStr, "LB");
			break;
		case 0x04:
			//left
			strcpy(cntStr, "HB");
			break;
		default:
			strcpy(cntStr, "  ");
			break;
		}
		RAMN_SPI_DrawStringColor(180,CONTROL_WINDOW_Y,SPI_SUBCONSOLE_COLORDYNAMIC,SPI_SUBCONSOLE_BACKGROUNDCOLOR,cntStr);
	}

	if ((RAMN_DBC_Handle.control_enginekey &0xFF) != prev_enginekey)
	{
		prev_enginekey = (RAMN_DBC_Handle.control_enginekey &0xFF);
		switch((RAMN_DBC_Handle.control_enginekey &0xFF))
		{
		case 0x01:
			strcpy(cntStr, "OFF");
			break;
		case 0x02:
			strcpy(cntStr, "ACC");
			break;
		case 0x03:
			strcpy(cntStr, "IGN");
			break;
		default:
			strcpy(cntStr, "   ");
			break;
		}
		RAMN_SPI_DrawStringColor(200,CONTROL_WINDOW_Y+16,SPI_SUBCONSOLE_COLORDYNAMIC,SPI_SUBCONSOLE_BACKGROUNDCOLOR,cntStr);
	}

}

void RAMN_ScreenUtils_DrawBase(uint8_t theme)
{
	SPI_COLOR_THEME.BACKGROUND = 0x0000;
	SPI_COLOR_THEME.WHITE = 0xFFFF;
	switch(theme)
	{
	case 0:
	case 1:
		SPI_COLOR_THEME.DARK = 0x01e0;
		SPI_COLOR_THEME.MEDIUM = 0x0462;
		SPI_COLOR_THEME.LIGHT = 0x07e8;
		break;
	case 2:
		SPI_COLOR_THEME.DARK = 0x210a;
		SPI_COLOR_THEME.MEDIUM = 0x31b2;
		SPI_COLOR_THEME.LIGHT = 0x52df;
		break;
	case 3:
		SPI_COLOR_THEME.DARK = 0x7145;
		SPI_COLOR_THEME.MEDIUM = 0xb1c7;
		SPI_COLOR_THEME.LIGHT = 0xf9c7;
		break;
	case 4:
		SPI_COLOR_THEME.DARK = 0x902a;
		SPI_COLOR_THEME.MEDIUM = 0xe02f;
		SPI_COLOR_THEME.LIGHT = 0xf8b2;
		break;
	case 5:
		SPI_COLOR_THEME.WHITE = 0x0000;
		SPI_COLOR_THEME.BACKGROUND = 0xFFFF;
		SPI_COLOR_THEME.DARK = 0x9c92;
		SPI_COLOR_THEME.MEDIUM = 0x6b4d;
		SPI_COLOR_THEME.LIGHT = 0x4208;
		break;
	case 7:
		SPI_COLOR_THEME.WHITE = 0x0000;
		SPI_COLOR_THEME.BACKGROUND = 0xFFFF;
	case 6:
		SPI_COLOR_THEME.DARK = 0x07e8;
		SPI_COLOR_THEME.MEDIUM = 0x52df;
		SPI_COLOR_THEME.LIGHT = 0xf9c7;
		break;
	default:
		SPI_COLOR_THEME.DARK = 0x01e0;
		SPI_COLOR_THEME.MEDIUM = 0x0462;
		SPI_COLOR_THEME.LIGHT = 0x07e8;
		break;
	}

	RAMN_SPI_DrawRectangle(0,0,LCD_WIDTH,LCD_HEIGHT-32,SPI_COLOR_THEME.BACKGROUND);
	RAMN_SPI_DrawContour(0, 0, LCD_WIDTH, LCD_HEIGHT-41, CONTOUR_WIDTH, SPI_COLOR_THEME.LIGHT);
	RAMN_ScreenUtils_DrawSubconsoleStatic();
	RAMN_CHIP8_SetColor(SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND);

}

void RAMN_ScreenUtils_UpdateTheme(uint8_t new_theme)
{
	if (new_theme != current_theme)
	{
		current_theme = new_theme;
		theme_change_requested = 1U;
		//TODO: ensure all screens redrawn
	}
}

void RAMN_ScreenUtils_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask)
{
	RAMN_SPI_Init(handler, pTask);
	RAMN_SPI_InitScreen();

	current_theme = RAMN_RNG_Pop8();
	if (current_theme > 127)
		current_theme = 1; // make theme 1 more likely, and don't select theme 5 to 7
	else  current_theme %= 5;

	RAMN_ScreenUtils_DrawBase(current_theme);

	//Init joystick for screen controls
	RAMN_Joystick_Init();
}

#endif
