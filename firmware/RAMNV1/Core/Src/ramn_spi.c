/*
 * ramn_spi.c
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

#include "ramn_spi.h"

#ifdef ENABLE_SPI
static SPI_HandleTypeDef* hspi;
osThreadId_t* pSPITask;
#endif

#if defined(ENABLE_SCREEN)
__attribute__ ((section (".buffers"))) static uint16_t spiTxBuffer[16*16];
static uint16_t currentScroll = SCREEN_HEADER_SIZE;
#endif

#ifdef ENABLE_SPI
void RAMN_SPI_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask)
{
	hspi = handler;
	pSPITask = pTask;
}
#endif

#if defined(ENABLE_SPI)

// Callback for End of SPI transmission
void HAL_SPI_TxCpltCallback (SPI_HandleTypeDef * hspi)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(*pSPITask,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

#endif

#ifdef ENABLE_SCREEN

static HAL_StatusTypeDef SPI_WriteData_DMA(const uint8_t *data, uint16_t nbytes)
{
	HAL_StatusTypeDef result;

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin , GPIO_PIN_SET);  // 1 for data, 0 for control
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_RESET);
	result = HAL_SPI_Transmit_DMA(hspi, (uint8_t*)data, nbytes);
	if (result == HAL_OK) ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_SET);
	return result;
}

static HAL_StatusTypeDef SPI_WriteCommand_DMA(uint8_t cmd)
{
	HAL_StatusTypeDef result;

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_RESET);
	result = HAL_SPI_Transmit_DMA(hspi, &cmd, 1);
	if (result == HAL_OK) ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_SET);
	return result;
}

static HAL_StatusTypeDef SPI_WriteDataUint32_DMA(uint32_t data)
{
	HAL_StatusTypeDef result;
	uint8_t buf[4];

	buf[0] = (data>>24)&0xFF;
	buf[1] = (data>>16)&0xFF;
	buf[2] = (data>>8)&0xFF;
	buf[3] = (data)&0xFF;
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_RESET);
	result = HAL_SPI_Transmit_DMA(hspi, buf, 4);
	if (result == HAL_OK) ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_SET);
	return result;
}

static void SPI_SetAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
	uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);
	SPI_WriteCommand_DMA(ST7789_CASET); // Set column
	SPI_WriteDataUint32_DMA(xa);

	SPI_WriteCommand_DMA(ST7789_RASET); // Set row
	SPI_WriteDataUint32_DMA(ya);

	SPI_WriteCommand_DMA(ST7789_RAMWR);
}

void RAMN_SPI_InitScreen(void)
{
	const uint8_t LCD_INIT_INVERSE = 1U;
	const uint8_t LCD_INIT_MADCTL[] = {0x00};
	const uint8_t LCD_INIT_COLMOD[] = {0x55};
	const uint8_t LCD_INIT_CASET[] = {(LCD_WIDTH)>>8, LCD_WIDTH&0xFF, 0, 0};
	const uint8_t LCD_INIT_RASET[] = {(LCD_HEIGHT)>>8, LCD_HEIGHT&0xFF, 0,0};
	const uint8_t LCD_INIT_RAMCTRL[] = {0x00,0xF8};
	const uint8_t LCD_INIT_VSCRDEF[] = {0x00, SCREEN_HEADER_SIZE, (SCROLL_AREA_SIZE >> 8)&0xFF, SCROLL_AREA_SIZE&0xFF, 0x00, SCREEN_FOOTER_SIZE};
	const uint8_t LCD_INIT_VSCSAD[] = {0x00, SCREEN_HEADER_SIZE};

	// Wait for SPI communications to be ready
	osDelay(500); //TODO: optimize this

	// Initialize Screen
	SPI_WriteCommand_DMA(ST7789_SWRESET); // Reset
	osDelay(50);

	SPI_WriteCommand_DMA(ST7789_SPLOUT); // Leave Sleep mode
	osDelay(50);

	if (LCD_INIT_INVERSE != 0U)	SPI_WriteCommand_DMA(ST7789_INVON); //Inverse Screen colors

	// Memory Data Access Control
	// Left to Right, Top to Bottom
	SPI_WriteCommand_DMA(ST7789_MADCTL);
	SPI_WriteData_DMA(LCD_INIT_MADCTL,sizeof(LCD_INIT_MADCTL));

	// Interface Pixel format
	// 65k of RGB, 16 bits / pixel
	SPI_WriteCommand_DMA(ST7789_COLMOD);
	SPI_WriteData_DMA(LCD_INIT_COLMOD,sizeof(LCD_INIT_COLMOD));

	// Column Address Set
	SPI_WriteCommand_DMA(ST7789_CASET);
	SPI_WriteData_DMA(LCD_INIT_CASET,sizeof(LCD_INIT_CASET));

	// Row Address Set
	SPI_WriteCommand_DMA(ST7789_RASET);
	SPI_WriteData_DMA(LCD_INIT_RASET,sizeof(LCD_INIT_RASET));

	// Turn ON
	SPI_WriteCommand_DMA(ST7789_NORON);

	// Turn ON display
	SPI_WriteCommand_DMA(ST7789_DISPON);

	// Use Little Endian
	SPI_WriteCommand_DMA(ST7789_RAMCTRL);
	SPI_WriteData_DMA(LCD_INIT_RAMCTRL,sizeof(LCD_INIT_RAMCTRL));

	SPI_WriteCommand_DMA(ST7789_VSCRDEF);
	SPI_WriteData_DMA(LCD_INIT_VSCRDEF, sizeof(LCD_INIT_VSCRDEF));

	SPI_WriteCommand_DMA(ST7789_VSCSAD);
	SPI_WriteData_DMA(LCD_INIT_VSCSAD, sizeof(LCD_INIT_VSCSAD));

	// Start writing data
	SPI_WriteCommand_DMA(ST7789_RAMWR);

}

// /* Example to scroll screen from a task */
// RAMN_SPI_SetScroll(SCREEN_HEADER_SIZE + ((tick/10)%(SCROLL_WINDOW_HEIGHT-SCREEN_HEADER_SIZE)));

void RAMN_SPI_SetScroll(uint16_t val)
{
	uint8_t v[2];

	currentScroll = val;
	v[0] = (currentScroll>>8)&0xFF;
	v[1] =currentScroll&0xFF;
	SPI_WriteCommand_DMA(ST7789_VSCSAD);
	SPI_WriteData_DMA(v,sizeof(v));
}

void RAMN_SPI_ScrollUp(uint16_t addVal)
{
	currentScroll = (currentScroll+addVal)%SCROLL_WINDOW_HEIGHT;
	if (currentScroll < SCREEN_HEADER_SIZE) currentScroll = SCREEN_HEADER_SIZE;
	RAMN_SPI_SetScroll(currentScroll);
}

void RAMN_SPI_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	SPI_SetAddrWindow(x,y,1,1);
	SPI_WriteData_DMA((uint8_t*)&color,2);
}

void RAMN_SPI_DrawRectangle(uint16_t startx, uint16_t starty, uint16_t w, uint16_t h,uint16_t color)
{
	uint32_t size = w*h;

	for(uint32_t i = 0; i < sizeof(spiTxBuffer)/2;i++)
	{
		spiTxBuffer[i] = color;
	}

	SPI_SetAddrWindow(startx,starty,w,h);

	for(uint32_t i = 0; i < size;i+=sizeof(spiTxBuffer)/2)
	{
		SPI_WriteData_DMA((uint8_t*)&spiTxBuffer, sizeof(spiTxBuffer));
	}
}

void RAMN_SPI_DrawContour(uint16_t startx, uint16_t starty, uint16_t endx, uint16_t endy, uint16_t width, uint16_t color)
{
	RAMN_SPI_DrawRectangle(startx,starty,endx-startx,width,color);
	RAMN_SPI_DrawRectangle(startx,starty,width,endy-starty,color);
	RAMN_SPI_DrawRectangle(startx,endy-width,endx-startx,width,color);
	RAMN_SPI_DrawRectangle(endx-width,starty,width,endy-starty,color);
}

// Image must be Little-Endian RGB565 format
void RAMN_SPI_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* image)
{
	uint32_t byteLen = 2 * (uint32_t)w * (uint32_t)h;
	uint32_t index = 0;

	SPI_SetAddrWindow(x,y,w,h);

	// DMA library only accepts a uint16_t size parameter, so we can only write with 16-bit blocks
	while (index < byteLen)
	{
		SPI_WriteData_DMA(image+index,((byteLen-index) <= 0xFFFF) ? (uint16_t) (byteLen-index) : 0xFFFF);
		index+= 0xFFFF;
	}
}

// Each char in the font array occupies 16*16 (with data that is always 0). This function draws all the font data (16*16).
void RAMN_SPI_DrawChar(uint16_t x, uint16_t y, uint16_t fgColor, uint16_t bgColor, uint8_t chr)
{
	uint8_t* array = (uint8_t*)&Font16.table[(chr - 0x20)*16*2];

	SPI_SetAddrWindow(x,y,16,16);
	for (uint16_t i = 0; i < 32; i+=2)
	{
		uint16_t val = (uint16_t)(array[i]<<8) + (uint16_t)array[i+1];
		for (uint16_t j = 0; j < 16; j++)
		{
			if (val&(1 << j)) spiTxBuffer[(i*8)+15-j] = fgColor;
			else spiTxBuffer[(i*8)+15-j] = bgColor;
		}
	}
	SPI_WriteData_DMA((uint8_t*)&spiTxBuffer,2*16*16);
}

// Each char in the font array occupies 16*16. This function only draws the important part (11*14).
void RAMN_SPI_RefreshChar(uint16_t x, uint16_t y, uint16_t fgColor, uint16_t bgColor, uint8_t chr)
{
	uint8_t* array = (uint8_t*)&Font16.table[(chr - 0x20)*16*2];

	SPI_SetAddrWindow(x,y,11,14);
	for (uint16_t i = 0; i < 14; i++)
	{
		uint16_t val = (uint16_t)(array[2*i]<<8) + (uint16_t)array[2*i+1];
		for (uint16_t j = 0; j < 11; j++)
		{
			if (val&(1 << (15-j))) spiTxBuffer[(i*11)+j] = fgColor;
			else spiTxBuffer[(i*11)+j] = bgColor;
		}
	}
	SPI_WriteData_DMA((uint8_t*)&spiTxBuffer,2*11*14);
}

void RAMN_SPI_DrawString(uint16_t x, uint16_t y, uint16_t fgColor, uint16_t bgColor, const char* src)
{
	uint16_t index = 0;
	uint16_t origin_x = x;

	while (src[index] != 0)
	{
		if ((src[index] == '\r') || (src[index] == '\n'))
		{
			x = origin_x;
			y += Font16.Height;
		}
		else
		{
			if (x > (LCD_WIDTH-16))
			{
				x = origin_x;
				y += Font16.Height;
			}
			RAMN_SPI_DrawChar(x,y,fgColor,bgColor,src[index]);
			x += Font16.Width;
		}
		index++;
	}
}

void RAMN_SPI_RefreshString(uint16_t x, uint16_t y, uint16_t fgColor, uint16_t bgColor, const char* src)
{
	uint16_t index = 0;
	uint16_t origin_x = x;

	while (src[index] != 0)
	{
		if ((src[index] == '\r') || (src[index] == '\n'))
		{
			x = origin_x;
			y += Font16.Height;
		}
		else
		{
			if (x > (LCD_WIDTH-16))
			{
				x = origin_x;
				y += Font16.Height;
			}
			RAMN_SPI_RefreshChar(x,y,fgColor,bgColor,src[index]);
			x += Font16.Width;
		}

		index++;
	}
}

void RAMN_SPI_DrawUint32(uint16_t x, uint16_t y, uint16_t fgColor, uint16_t bgColor, uint32_t val)
{
	uint8_t tmp[9];
	tmp[8] = 0; // Terminate string
	uint32toASCII(val, tmp);
	RAMN_SPI_DrawString(x, y, fgColor, bgColor, (char*)tmp);
}

void RAMN_SPI_RefreshUint32(uint16_t x, uint16_t y, uint16_t fgColor, uint16_t bgColor, uint32_t val)
{
	uint8_t tmp[9];
	tmp[8] = 0; // Terminate string
	uint32toASCII(val, tmp);
	RAMN_SPI_RefreshString(x, y, fgColor, bgColor, (char*)tmp);
}

#endif

#if defined(EXPANSION_BODY)
void RAMN_SPI_UpdateLED(uint8_t* pval)
{
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_RESET );
	HAL_SPI_Transmit_IT(hspi, pval, 1);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_SET);
}

#endif
