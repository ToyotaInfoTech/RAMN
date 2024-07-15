/*
 * ramn_screen_stats.c
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


#include "ramn_screen_stats.h"

#ifdef ENABLE_SCREEN


static void ScreenStats_Init() {
	RAMN_ScreenUtils_DrawBase(current_theme);
	RAMN_SPI_DrawStringColor2(90,5, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "STATS");
	RAMN_SPI_DrawStringColor2(7,5+16, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "CAN RX  : 0x");
	RAMN_SPI_DrawStringColor2(7,5+32, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "CAN TX  : 0x");
	RAMN_SPI_DrawStringColor2(7,5+48, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "CAN ERR : 0x");
	RAMN_SPI_DrawStringColor2(7,5+64, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "CAN OVF :");
	RAMN_SPI_DrawStringColor2(7,5+80, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "CAN BUS :");
	RAMN_SPI_DrawStringColor2(7,5+112, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "USB ERR : 0x");
	RAMN_SPI_DrawStringColor2(7,5+128, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "USB OVF : 0x");
	RAMN_SPI_DrawStringColor2(7,5+160, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "SYSTICK : 0x");

}

static void ScreenStats_Update(uint32_t tick) {

	uint8_t tmp[9];
	tmp[8] = 0;

	if (spi_refresh_counter % 5 == 0)
	{
		uint32toASCII(RAMN_FDCAN_Status.CANRXCnt, tmp);
		RAMN_SPI_DrawStringColor2(7+12*11,5+16, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, tmp);


		uint32toASCII(RAMN_FDCAN_Status.CANTXSentCnt, tmp);
		RAMN_SPI_DrawStringColor2(7+12*11,5+32, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, tmp);

		uint32toASCII(RAMN_FDCAN_Status.CANErrCnt, tmp);
		if (RAMN_FDCAN_Status.CANErrCnt > 0)
		{
			RAMN_SPI_DrawStringColor2(7+12*11,5+48, SPI_COLOR_THEME.WHITE, SPI_COLOR_THEME.BACKGROUND, tmp);
		}
		else
		{
			RAMN_SPI_DrawStringColor2(7+12*11,5+48, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, tmp);
		}
		if (RAMN_FDCAN_Status.CANRxOverrunCnt > 0)
		{
			RAMN_SPI_DrawStringColor2(7+10*11,5+64, SPI_COLOR_THEME.WHITE, SPI_COLOR_THEME.BACKGROUND, "YES");
		}
		else
		{
			RAMN_SPI_DrawStringColor2(7+10*11,5+64, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "NO");
		}

		if (RAMN_FDCAN_Status.busOff != False)
		{
			RAMN_SPI_DrawStringColor2(7+10*11,5+80, SPI_COLOR_THEME.WHITE, SPI_COLOR_THEME.BACKGROUND, "OFF");
		}
		else
		{
			RAMN_SPI_DrawStringColor2(7+10*11,5+80, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, "ON");
		}

		uint32toASCII(RAMN_USB_Config.USBErrCnt, tmp);
		if (RAMN_USB_Config.USBErrCnt > 0)
		{
			RAMN_SPI_DrawStringColor2(7+12*11,5+112, SPI_COLOR_THEME.WHITE, SPI_COLOR_THEME.BACKGROUND, tmp);
		}
		else
		{
			RAMN_SPI_DrawStringColor2(7+12*11,5+112, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, tmp);
		}


		uint32toASCII(RAMN_USB_Config.USBTxOverflowCnt, tmp);
		if (RAMN_USB_Config.USBTxOverflowCnt > 0)
		{
			RAMN_SPI_DrawStringColor2(7+12*11,5+128, SPI_COLOR_THEME.WHITE, SPI_COLOR_THEME.BACKGROUND, tmp);
		}
		else
		{
			RAMN_SPI_DrawStringColor2(7+12*11,5+128, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, tmp);
		}

		uint32toASCII(tick, tmp);
		RAMN_SPI_DrawStringColor2(7+12*11,5+160, SPI_COLOR_THEME.LIGHT, SPI_COLOR_THEME.BACKGROUND, tmp);

		RAMN_ScreenUtils_DrawSubconsoleUpdate();
	}


}

static void ScreenStats_Deinit() {
}

static void ScreenStats_UpdateInput(JoystickEventType event) {
}

RAMNScreen ScreenStats = {
		.Init = ScreenStats_Init,
		.Update = ScreenStats_Update,
		.Deinit = ScreenStats_Deinit,
		.UpdateInput = ScreenStats_UpdateInput
};

#endif
