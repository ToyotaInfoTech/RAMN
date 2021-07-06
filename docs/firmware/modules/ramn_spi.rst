SPI Peripheral Handling
=======================

Description
-----------

The **ramn_spi** module controls SPI communication, which is used both for ECU A's screen and the BODY expansion's LEDs.

.. warning:: Since the module relies on the DMA for data transmission, it can only be called by one task, which handle should be provided in the Init function.

.. code-block:: C

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

	//Updates the Screen. Must be called periodically
	void	RAMN_SPI_UpdateScreen(uint32_t tick);