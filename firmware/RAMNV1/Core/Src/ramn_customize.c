/*
 * ramn_customize.c
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

#include "ramn_customize.h"

uint32_t custom_loop_counter = 0;

void 	RAMN_CUSTOM_Init(uint32_t tick)
{
	custom_loop_counter = 0;
}

void	RAMN_CUSTOM_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick)
{
	//Fields that you may want to use:
	//pHeader->Identifier: (11-bit val for standard, 29-bit for extended)
	//pHeader->IdType: FDCAN_STANDARD_ID or FDCAN_EXTENDED_ID
	//pHeader->RxFrameType:  FDCAN_DATA_FRAME or FDCAN_REMOTE_FRAME
	//DataLength: length of CAN payload, FDCAN_DLC_BYTES_0 (0) to FDCAN_DLC_BYTES_8 (8) for CAN, FDCAN_DLC_BYTES_0 (0) to FDCAN_DLC_BYTES_64 (0xF, Not 64) for CAN-FD.
	//pHeader->ErrorStateIndicator: For CAN-FD, either FDCAN_ESI_ACTIVE or FDCAN_ESI_PASSIVE
	//pHeader->BitRateSwitch: For CAN-FD, either FDCAN_BRS_OFF or FDCAN_BRS_ON
	//pHeader->FDFormat: FDCAN_CLASSIC_CAN or FDCAN_FD_CAN
	//pHeader->RxTimestamp: 16-bit value for RX timestamp, MAY NOT BE CONFIGURED CORRECTLY
	//See FilterIndex and IsFilterMatchingFrame for additional fields.
}

void RAMN_CUSTOM_Update(uint32_t tick)
{
	//This function is called by a dedicated periodic task, which means code can here won't block other functionalities (such as receiving CAN messages).
	//Modify SIM_LOOP_CLOCK_MS if you want to use another period than 10ms.

	//Code here is executed every 10ms

	if ((custom_loop_counter % 10) == 0)
	{
		//Code here is executed every 100ms
	}

	if ((custom_loop_counter % 100) == 0)
	{
		//Code here is executed every 1s
	}

	custom_loop_counter += 1; 	//You may want to add a custom_loop_counter check for integer overflow.

}

#ifdef ENABLE_I2C
void RAMN_CUSTOM_ReceiveI2C(uint8_t buf[], uint16_t buf_size)
{
// ** FUNCTION CURRENTLY NOT FULLY TESTED **

//Warning: This function is called within an ISR. It should not use freeRTOS functions not available to ISRs, and should not be blocking.
//See RAMNV1.ioc for I2C device address (likely 0x77)
//Note that by default, buf_size is fixed and equal to I2C_RX_BUFFER_SIZE. Function will NOT be called if fewer bytes are received.
//You'll need to modify HAL_I2C_AddrCallback and HAL_I2C_SlaveRxCpltCallback in main.c if you need another behavior.

}

void RAMN_CUSTOM_PrepareTransmitDataI2C(uint8_t buf[], uint16_t buf_size)
{
// ** FUNCTION CURRENTLY NOT FULLY TESTED **

	//Warning: This function is called within an ISR. It should not use freeRTOS functions not available to ISRs, and should not be blocking.
//Note that you cannot modify buf_size, only buf.
//You'll need to modify HAL_I2C_AddrCallback in main.c if you need another behavior.
}
#endif
