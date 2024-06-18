/*
 * ramn_spi.c
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

#include "ramn_spi.h"

#if defined(ENABLE_SCREEN) || defined(EXPANSION_BODY)
static SPI_HandleTypeDef* hspi;
osThreadId_t* pSPITask;
#endif

#if defined(ENABLE_SCREEN)
static uint16_t spiTxBuffer[16*16];
#endif

#if defined(ENABLE_SCREEN) || defined(EXPANSION_BODY)
void RAMN_SPI_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask)
{
	hspi = handler;
	pSPITask = pTask;
}
#endif

#if defined(ENABLE_SCREEN)
static void writeData_DMA(const uint8_t *data, uint16_t nbytes)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin , GPIO_PIN_SET);  // 1 for data, 0 for control
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_RESET );
	HAL_SPI_Transmit_DMA(hspi, (uint8_t*)data, nbytes);
	ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_SET);
}

static void writeCommand_DMA(uint8_t cmd)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_RESET );
	HAL_SPI_Transmit_DMA(hspi, &cmd, 1);
	ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_SET);
}

static void writeDataUint32_DMA(uint32_t data)
{
	uint8_t buf[4];
	buf[0] = (data>>24)&0xFF;
	buf[1] = (data>>16)&0xFF;
	buf[2] = (data>>8)&0xFF;
	buf[3] = (data)&0xFF;
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_RESET );
	HAL_SPI_Transmit_DMA(hspi, buf, 4);
	ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_SET);
}

static void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
	uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);
	writeCommand_DMA(ST7789_CASET); // Column addr set
	writeDataUint32_DMA(xa);

	writeCommand_DMA(ST7789_RASET); // Row addr set
	writeDataUint32_DMA(ya);

	writeCommand_DMA(ST7789_RAMWR);
}

void HAL_SPI_TxCpltCallback (SPI_HandleTypeDef * hspi)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(*pSPITask,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


void RAMN_SPI_SetScroll(uint16_t val)
{
	writeCommand_DMA(ST7789_VSCSAD);
	uint8_t v[2];
	v[0] = (val>>8)&0xFF;
	v[1] =val&0xFF;
	writeData_DMA(v,sizeof(v));
}

void RAMN_SPI_InitScreen(void)
{
	const uint8_t LCD_INIT_INVERSE = 1U;
	//const uint8_t LCD_INIT_MADCTL[] = {0x00};  //For RAMN 2layers
	const uint8_t LCD_INIT_MADCTL[] = {0x00};
	const uint8_t LCD_INIT_COLMOD[] = {0x55};
//	const uint8_t LCD_INIT_CASET[] = {0x00, 0x00, (LCD_WIDTH)>>8, LCD_WIDTH&0xFF};
//	const uint8_t LCD_INIT_RASET[] = {0x00, 0x00, (LCD_HEIGHT)>>8, LCD_HEIGHT&0xFF };
	const uint8_t LCD_INIT_CASET[] = {(LCD_WIDTH)>>8, LCD_WIDTH&0xFF, 0, 0};
	const uint8_t LCD_INIT_RASET[] = {(LCD_HEIGHT)>>8, LCD_HEIGHT&0xFF, 0,0};
	const uint8_t LCD_INIT_RAMCTRL[] = {0x00,0xF8};

	//const uint8_t LCD_INIT_VSCRDEF[] = {0x00,0x4, 0x00, 0xE8, 0x00, 0x4};
	const uint8_t LCD_INIT_VSCRDEF[] = {0x00, SCREEN_HEADER_SIZE, 0x00, LCD_HEIGHT-SCREEN_HEADER_SIZE, 0x00,0x00};
	const uint8_t LCD_INIT_VSCSAD[] = {0x00, SCREEN_HEADER_SIZE};


	//Wait for SPI communications to be ready
	osDelay(500); //TODO: optimize this

	//Initialize Screen
	writeCommand_DMA(ST7789_SWRESET); //Reset
	osDelay(50);

	writeCommand_DMA(ST7789_SPLOUT); //Leave Sleep mode
	osDelay(50);

	if (LCD_INIT_INVERSE != 0U)	writeCommand_DMA(ST7789_INVON); //Inverse Screen colors

	//Memory Data Access Control
	//Left to Right, Top to Bottom

	writeCommand_DMA(ST7789_MADCTL);
	writeData_DMA(LCD_INIT_MADCTL,sizeof(LCD_INIT_MADCTL));

	//Interface Pixel format
	//65k of RGB, 16 bits / pixel
	writeCommand_DMA(ST7789_COLMOD);
	writeData_DMA(LCD_INIT_COLMOD,sizeof(LCD_INIT_COLMOD));

	//Column Address Set
	writeCommand_DMA(ST7789_CASET);
	writeData_DMA(LCD_INIT_CASET,sizeof(LCD_INIT_CASET));

	//Row Address Set
	writeCommand_DMA(ST7789_RASET);
	writeData_DMA(LCD_INIT_RASET,sizeof(LCD_INIT_RASET));

	//Turn ON
	writeCommand_DMA(ST7789_NORON);

	//Turn ON display
	writeCommand_DMA(ST7789_DISPON);

	//Set Endian to Little Endian
	writeCommand_DMA(ST7789_RAMCTRL);
	writeData_DMA(LCD_INIT_RAMCTRL,sizeof(LCD_INIT_RAMCTRL));

	writeCommand_DMA(ST7789_VSCRDEF);
	writeData_DMA(LCD_INIT_VSCRDEF, sizeof(LCD_INIT_VSCRDEF));

	writeCommand_DMA(ST7789_VSCSAD);
	writeData_DMA(LCD_INIT_VSCSAD, sizeof(LCD_INIT_VSCSAD));

	//Start writing data
	writeCommand_DMA(ST7789_RAMWR);

}

void RAMN_SPI_DrawPixel(uint16_t x, uint16_t y, uint16_t pixel16)
{
	setAddrWindow(x,y,1,1);
	writeData_DMA((uint8_t*)&pixel16,2);
}

void RAMN_SPI_DrawRectangle(uint16_t startx, uint16_t starty, uint16_t w, uint16_t h,uint16_t color)
{
	uint32_t size = w*h;
	for(uint32_t i = 0; i < sizeof(spiTxBuffer)/2;i++)
	{
		spiTxBuffer[i] = color;
	}
	setAddrWindow(startx,starty,w,h);
	for(uint32_t i = 0; i < size;i+=sizeof(spiTxBuffer)/2)
	{
		writeData_DMA((uint8_t*)&spiTxBuffer, sizeof(spiTxBuffer));
	}
}

void RAMN_SPI_DrawContour(uint16_t startx, uint16_t starty, uint16_t endx, uint16_t endy, uint16_t width, uint16_t color)
{
	RAMN_SPI_DrawRectangle(startx,starty,endx-startx,width,color);
	RAMN_SPI_DrawRectangle(startx,starty,width,endy-starty,color);
	RAMN_SPI_DrawRectangle(startx,endy-width,endx-startx,width,color);
	RAMN_SPI_DrawRectangle(endx-width,starty,width,endy-starty,color);
}

//image must be Little-Endian RGB565 format
void RAMN_SPI_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* image)
{
	uint32_t byteLen = 2 * (uint32_t)w * (uint32_t)h;
	uint32_t index = 0;
	setAddrWindow(x,y,w,h);
	//Can only write with 16-bit blocks
	while (index < byteLen)
	{
		writeData_DMA(image+index,((byteLen-index) <= 0xFFFF) ? (uint16_t) (byteLen-index) : 0xFFFF);
		index+= 0xFFFF;
	}
}

void RAMN_SPI_DrawCharColor(uint16_t x, uint16_t y, uint16_t fg_color, uint16_t bg_color, uint8_t chr)
{
	uint8_t* array = (uint8_t*)&Font16.table[(chr - 0x20)*16*2];
	setAddrWindow(x,y,16,16);
	for (uint16_t i = 0; i < 32; i+=2)
	{
		uint16_t val = (uint16_t)(array[i]<<8) + (uint16_t)array[i+1];
		for (uint16_t j = 0; j < 16; j++)
		{
			if (val&(1 << j)) spiTxBuffer[(i*8)+15-j] = fg_color;
			else spiTxBuffer[(i*8)+15-j] = bg_color;
		}
	}
	writeData_DMA((uint8_t*)&spiTxBuffer,2*16*16);
}

void RAMN_SPI_DrawChar(uint16_t x, uint16_t y, uint8_t chr)
{
	RAMN_SPI_DrawCharColor(x,y,COLOR_WHITE,COLOR_BLACK,chr);
}

void RAMN_SPI_DrawStringColor(uint16_t x, uint16_t y, uint16_t fg_color, uint16_t bg_color, const char* src)
{
	uint16_t index = 0;
	uint16_t origin_x;
	origin_x = x;
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
			RAMN_SPI_DrawCharColor(x,y,fg_color,bg_color,src[index]);
			x += Font16.Width;
		}

		index++;
	}

}

void RAMN_SPI_DrawString(uint16_t x, uint16_t y, char* src)
{
	RAMN_SPI_DrawStringColor(x,y,COLOR_WHITE,COLOR_BLACK,src);
}

void RAMN_SPI_RefreshStringColor(uint16_t x, uint16_t y, uint16_t fg_color, uint16_t bg_color, const char* src, char* prev)
{
	uint16_t index = 0;
	uint8_t needRefresh = 0U;
	while (src[index] != 0)
	{

		if ((src[index] == '\r') || (src[index] == '\n'))
		{
			x = 0;
			y += Font16.Height;
		}
		else
		{
			if (src[index] != prev[index] || (needRefresh != 0U))
			{
				needRefresh = 1U;
				RAMN_SPI_DrawCharColor(x,y,fg_color,bg_color,src[index]);
				prev[index] = src[index];
			}
			x += Font16.Width;
			if (x > (LCD_WIDTH-16))
			{
				x = 0;
				y += Font16.Height;
			}
		}

		if (y > (LCD_HEIGHT - 16))
		{
			x = 0;
			y = 0;
		}
		index++;

	}
}

#endif

#if defined(EXPANSION_BODY)
void RAMN_SPI_UpdateLED(uint8_t* pval)
{
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_RESET );
	HAL_SPI_Transmit_IT(hspi, pval, 1); //Try to transmit but don't insist
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port, LCD_nCS_Pin, GPIO_PIN_SET);
}

#endif
