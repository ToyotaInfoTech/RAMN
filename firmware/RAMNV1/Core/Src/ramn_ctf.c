/*
 * ramn_ctf.c
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

#include "ramn_ctf.h"
#include <string.h>

#ifdef ENABLE_MINICTF

#if defined(TARGET_ECUD)

static uint32_t ctfLoopCounter;

// TODO: put these variables in stack
// variables to answer immediately when a message is received
static FDCAN_TxHeaderTypeDef CTFTxHeader;
static uint8_t CTFTxData[8U];

// variables to transmit messages periodically
static FDCAN_TxHeaderTypeDef CTFTxHeaderPeriodic;
static uint8_t CTFTxDataPeriodic[8U];

static RAMN_Bool_t periodicFlagEnabled = False;

static void sendFlagOverCAN(uint16_t can_id, char* flag)
{
	CTFTxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	CTFTxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	CTFTxHeader.TxFrameType = FDCAN_DATA_FRAME;
	CTFTxHeader.IdType = FDCAN_STANDARD_ID;
	CTFTxHeader.Identifier = can_id;

	uint16_t size = RAMN_strlen(flag);
	uint16_t offset = 0;
	while (offset < size)
	{
		uint8_t dlc = 0;
		if ((size - offset) > 8U) dlc = 8U;
		else dlc = (uint8_t) (size - offset);

		CTFTxHeader.DataLength = UINT8toDLC(dlc);
		RAMN_memcpy((uint8_t*)CTFTxData,(uint8_t*)&flag[offset],dlc);
		RAMN_FDCAN_SendMessage(&CTFTxHeader,CTFTxData);
		offset += dlc;
	}
}

static uint8_t checkIfShouldSendFlag4(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data)
{
	if (pHeader->RxFrameType == FDCAN_DATA_FRAME)
	{
		if (pHeader->Identifier == CTF_EXTENDED_ID)
		{
			if ((data[0] == 'P') && (data[1] == '4') && (data[2] == '$') && (data[3] == '$') && (data[4] == 'W') && (data[5] == '0') && (data[6] == 'R') && (data[7] == 'D')) return 1U;
		}
	}
	return 0U;
}


static uint16_t offset_flag5 = 0;
static void SendNextDataFlag5()
{
	uint16_t size = (sizeof(FLAG_CAN_5)-1)*8;
	CTFTxHeaderPeriodic.BitRateSwitch = FDCAN_BRS_OFF;
	CTFTxHeaderPeriodic.FDFormat = FDCAN_CLASSIC_CAN;
	CTFTxHeaderPeriodic.TxFrameType = FDCAN_DATA_FRAME;
	CTFTxHeaderPeriodic.IdType = FDCAN_STANDARD_ID;
	CTFTxHeaderPeriodic.Identifier = 0x6F0;
	if (offset_flag5 == 0)
	{
		CTFTxHeaderPeriodic.DataLength = UINT8toDLC(2U);
		CTFTxDataPeriodic[1U] = 0U;
	}
	else
	{
		CTFTxHeaderPeriodic.DataLength = UINT8toDLC(1U);
	}
	CTFTxDataPeriodic[0U] = ((FLAG_CAN_5[(offset_flag5/8)]) >> (7 - (offset_flag5%8)))&1U;
	offset_flag5 += 1;
	if (offset_flag5 > size) offset_flag5 = 0U;

	RAMN_FDCAN_SendMessage(&CTFTxHeaderPeriodic,CTFTxDataPeriodic);

}

static uint16_t offset_flag6 = 0;
static void SendNextDataFlag6()
{
	uint16_t size = (sizeof(FLAG_CAN_6)-1)*8;
	CTFTxHeaderPeriodic.BitRateSwitch = FDCAN_BRS_OFF;
	CTFTxHeaderPeriodic.FDFormat = FDCAN_CLASSIC_CAN;
	CTFTxHeaderPeriodic.TxFrameType = FDCAN_DATA_FRAME;
	CTFTxHeaderPeriodic.IdType = FDCAN_STANDARD_ID;
	CTFTxHeaderPeriodic.Identifier = 0x6F1;
	if (offset_flag6 == 0)
	{
		CTFTxHeaderPeriodic.DataLength = UINT8toDLC(1U);
		CTFTxDataPeriodic[0U] = 0U;
	}
	else
	{
		CTFTxHeaderPeriodic.DataLength = UINT8toDLC(0U);
	}

	if (((((FLAG_CAN_6[(offset_flag6/8)]) >> (7 - (offset_flag6%8)))&1U)) == 0U)
	{
		CTFTxHeaderPeriodic.TxFrameType = FDCAN_DATA_FRAME;
	}
	else
	{
		CTFTxHeaderPeriodic.TxFrameType = FDCAN_REMOTE_FRAME;

	}
	offset_flag6 += 1;
	if (offset_flag6 > size) offset_flag6 = 0U;

	RAMN_FDCAN_SendMessage(&CTFTxHeaderPeriodic,CTFTxDataPeriodic);

}

static uint16_t offset_flag7 = 0;
static void SendNextDataFlag7()
{
	uint16_t size = (sizeof(FLAG_CAN_7)-1)*8;
	CTFTxHeaderPeriodic.BitRateSwitch = FDCAN_BRS_OFF;
	CTFTxHeaderPeriodic.FDFormat = FDCAN_CLASSIC_CAN;
	CTFTxHeaderPeriodic.TxFrameType = FDCAN_DATA_FRAME;
	CTFTxHeaderPeriodic.IdType = FDCAN_STANDARD_ID;
	CTFTxHeaderPeriodic.Identifier = 0x6F2;

	while (True)
	{
		if (offset_flag7 == 0)
		{
			CTFTxHeaderPeriodic.DataLength = UINT8toDLC(1U);
			CTFTxDataPeriodic[0U] = 0U;
		}
		else
		{
			CTFTxHeaderPeriodic.DataLength = UINT8toDLC(0U);
		}

		RAMN_FDCAN_SendMessage(&CTFTxHeaderPeriodic,CTFTxDataPeriodic);
		offset_flag7 += 1;
		if (offset_flag7 > size) offset_flag7 = 0U;

		if (((((FLAG_CAN_7[(offset_flag7/8)]) >> (7 - (offset_flag7%8)))&1U)) == 0U)
		{
			break;
		}
	}
}

#endif

void 	RAMN_CTF_Init(uint32_t tick)
{
#ifdef TARGET_ECUD
	ctfLoopCounter = 0;
#endif
}


void	RAMN_CTF_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick)
{
#ifdef TARGET_ECUD
	if (pHeader->RxFrameType == FDCAN_DATA_FRAME)
	{
		if (pHeader->Identifier == CTF_STANDARD_ID_1)
		{
			sendFlagOverCAN(0x770, FLAG_CAN_1);
		}
	}

	if (pHeader->RxFrameType == FDCAN_REMOTE_FRAME)
	{
		if (pHeader->Identifier == CTF_STANDARD_ID_2)
		{
			sendFlagOverCAN(0x772, FLAG_CAN_2);
		}
	}

	if (pHeader->RxFrameType == FDCAN_DATA_FRAME)
	{
		if (pHeader->Identifier == CTF_STANDARD_ID_3)
		{
			if (memcmp(data,"GIVEFLAG",8) == 0) sendFlagOverCAN(0x771, FLAG_CAN_3);
		}
	}

	if (checkIfShouldSendFlag4(pHeader,data) != 0U) sendFlagOverCAN(0x773, FLAG_CAN_4);

	if (pHeader->RxFrameType == FDCAN_DATA_FRAME)
	{
		if (pHeader->Identifier == CTF_STANDARD_ID_4)
		{
			if (pHeader->DataLength == 0)
			{
				periodicFlagEnabled = True;
				ctfLoopCounter = 0U;
			}
		}
	}
#endif
}

void RAMN_CTF_Update(uint32_t tick)
{
#ifdef TARGET_ECUD
	if (periodicFlagEnabled)
	{
		if ((ctfLoopCounter % 10) == 0)
		{
			SendNextDataFlag5();
			SendNextDataFlag6();
		}

		if ((ctfLoopCounter % 100) == 0)
		{
			SendNextDataFlag7();

		}

		ctfLoopCounter += 1;
	}
#endif
}
#endif
