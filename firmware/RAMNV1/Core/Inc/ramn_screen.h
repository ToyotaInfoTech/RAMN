/*
 * ramn_screen.h
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

// This module handles whatever is displayed on the screen

#ifndef INC_RAMN_SCREEN_H_
#define INC_RAMN_SCREEN_H_

#include "main.h"
#include "ramn_spi.h"
#include "ramn_dbc.h"

#define CONTOUR_WIDTH 2
#define CONTROL_WINDOW_Y LCD_HEIGHT-34

typedef struct COLOR_THEME_STRUCT {
    uint16_t BACKGROUND;
    uint16_t DARK;
    uint16_t MEDIUM;
    uint16_t LIGHT;
    uint16_t WHITE;
} ColorTheme_t ;

//Inits the Screen
void RAMN_Screen_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask);

//Updates the Screen. Must be called periodically
void	RAMN_Screen_Update(uint32_t tick);

//Update the color theme
void RAMN_Screen_UpdateTheme(uint8_t new_theme);

#endif /* INC_RAMN_SCREEN_H_ */
