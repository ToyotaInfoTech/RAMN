/*
 * ramn_spi.h
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 TOYOTA MOTOR CORPORATION.
  * ALL RIGHTS RESERVED.</center></h2>
  *
  * This software component is licensed by TOYOTA MOTOR CORPORATION under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
 */

// This Module handles SPI communication, with the LCD Screen (ECU A) or the LEDs (ECU D)

#ifndef INC_RAMN_SPI_H_
#define INC_RAMN_SPI_H_

#include "main.h"
#include "cmsis_os.h"
#ifdef ENABLE_SCREEN
#include "fonts.h"
#include "ramn_usb.h"
#include "ramn_canfd.h"
#include "ramn_trng.h"

//Below are definition for ST7789 Chipset screen
#define ST7789_CASET 0x2A
#define ST7789_RASET 0x2B
#define ST7789_MADCTL 0x36
#define ST7789_RAMWR 0x2C
#define ST7789_RAMRD 0x2E
#define ST7789_COLMOD 0x3A
#define ST7789_INVON  0x21
#define ST7789_SPLOUT 0x11
#define ST7789_SWRESET 0x01
#define ST7789_NORON 0x13
#define ST7789_DISPON 0x29
#define ST7789_RAMCTRL 0xB0
#define ST7789 SPI2EN 0xE7

#define LCD_WIDTH 240
#define LCD_HEIGHT 240
//#define LCD_HEIGHT 320

//Colors must be in RGB565 format
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F

#endif

#if defined(ENABLE_SCREEN) || defined(EXPANSION_BODY)
//Callback for SPI peripheral (End of transmission)
void 	HAL_SPI_TxCpltCallback(SPI_HandleTypeDef * hspi);

//Initializes the Module
void 	RAMN_SPI_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask);
#endif

#ifdef ENABLE_SCREEN
//Initializes the screen and turn it ON
void 	RAMN_SPI_InitScreen(void);

//Updates the value of a specific pixel on the screen
void 	RAMN_SPI_DrawPixel(uint16_t x, uint16_t y, uint16_t pixel16);

//Draws a rectangle of the specified color on the screen
void 	RAMN_SPI_DrawRectangle(uint16_t startx, uint16_t starty, uint16_t w, uint16_t h,uint16_t color);

//Draws a contour (empty rectangle)
void RAMN_SPI_DrawContour(uint8_t startx, uint8_t starty, uint8_t endx, uint8_t endy, uint8_t width, uint16_t color);

//Draws a 565 color image from specified array (Little Endian)
void 	RAMN_SPI_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* image);

//Draws ONE character with specified background/foreground color
void 	RAMN_SPI_DrawCharColor(uint16_t x, uint16_t y, uint16_t fg_color, uint16_t bg_color, uint8_t chr);

//Draws ONE character with default background/foreground color
void 	RAMN_SPI_DrawChar(uint16_t x, uint16_t y, uint8_t chr);

//Draws specified zero-terminated string with specified background/foreground color. \n and \r treated as newLines.
void 	RAMN_SPI_DrawStringColor(uint16_t x, uint16_t y, uint16_t fg_color, uint16_t bg_color, const char* src);

//Draws specified zero-terminated string with default background/foreground color. \n and \r treated as newLines.
void 	RAMN_SPI_DrawString(uint16_t x, uint16_t y, char* src);

//Function that updates a string by only drawing characters that changed
void RAMN_SPI_RefreshStringColor(uint16_t x, uint16_t y, uint16_t fg_color, uint16_t bg_color, const char* src, char* prev);

#endif

#ifdef EXPANSION_BODY
//Updates the value of the LED Shift register
void RAMN_SPI_UpdateLED(uint8_t* pval);
#endif

#endif /* INC_RAMN_SPI_H_ */
