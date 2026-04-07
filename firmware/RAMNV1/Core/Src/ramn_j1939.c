/*
 * ramn_j1939.c
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

#include "ramn_j1939.h"
#include "ramn_canfd.h"
#include "ramn_dbc.h"
#include "ramn_dtc.h"
#include "ramn_utils.h"

#ifdef ENABLE_J1939_MODE

/* ---------- J1939 helper: extract fields from 29-bit extended CAN ID ------ */
static uint8_t J1939_GetPF(uint32_t canId)  { return (uint8_t)((canId >> 16) & 0xFFU); }
static uint8_t J1939_GetPS(uint32_t canId)  { return (uint8_t)((canId >> 8)  & 0xFFU); }
static uint8_t J1939_GetSA(uint32_t canId)  { return (uint8_t)(canId & 0xFFU); }

/* Build a 29-bit J1939 CAN ID (EDP=0, DP=0) */
static uint32_t J1939_MakeId(uint8_t prio, uint8_t pf, uint8_t da, uint8_t sa)
{
	return ((uint32_t)(prio & 0x7U) << 26) | ((uint32_t)pf << 16) | ((uint32_t)da << 8) | (uint32_t)sa;
}

/*
 * J1939 NAME (8 bytes).
 */
static const uint8_t j1939_name[8] = {
#if defined(TARGET_ECUA)
	0x10, 0x02, J1939_ECU_FUNCTION, 0x00, 0x0F, 0xA0, 0x00, 0x01  /* Identity=1, MC=125, Function=32 (Headway Ctrl) */
#elif defined(TARGET_ECUB)
	0x10, 0x02, J1939_ECU_FUNCTION, 0x00, 0x3F, 0x60, 0x00, 0x02  /* Identity=2, MC=507, Function=16 (Steering Ctrl) */
#elif defined(TARGET_ECUC)
	0x10, 0x02, J1939_ECU_FUNCTION, 0x00, 0x41, 0x40, 0x00, 0x03  /* Identity=3, MC=522, Function=80 (Powertrain Ctrl) */
#elif defined(TARGET_ECUD)
	0x10, 0x02, J1939_ECU_FUNCTION, 0x00, 0x1F, 0x60, 0x00, 0x04  /* Identity=4, MC=251, Function=26 (Body Ctrl) */
#endif
};

/* ECU Identification string for PGN 64965 (fields delimited by '*') */
static const char j1939_ecu_id[] =
#if defined(TARGET_ECUA)
	"RAMN*ECU_A*0001*UNIT1*";
#elif defined(TARGET_ECUB)
	"RAMN*ECU_B*0002*UNIT2*";
#elif defined(TARGET_ECUC)
	"RAMN*ECU_C*0003*UNIT3*";
#elif defined(TARGET_ECUD)
	"RAMN*ECU_D*0004*UNIT4*";
#else
	"RAMN*ECU_UNKNOWN*";
#endif

/* Initialise common fields of a J1939 CAN TX header */
static void J1939_InitTxHeader(FDCAN_TxHeaderTypeDef *h, uint32_t canId)
{
	h->Identifier           = canId;
	h->IdType               = FDCAN_EXTENDED_ID;
	h->TxFrameType          = FDCAN_DATA_FRAME;
	h->DataLength           = FDCAN_DLC_BYTES_8;
	h->ErrorStateIndicator  = FDCAN_ESI_ACTIVE;
	h->BitRateSwitch        = FDCAN_BRS_OFF;
	h->FDFormat             = FDCAN_CLASSIC_CAN;
	h->TxEventFifoControl   = FDCAN_NO_TX_EVENTS;
	h->MessageMarker        = 0;
}

/* Send Address Claimed (PGN 60928) – 8-byte NAME payload */
static void J1939_SendAddressClaimed(uint8_t da)
{
	FDCAN_TxHeaderTypeDef header;
	J1939_InitTxHeader(&header, J1939_MakeId(6, J1939_PF_ADDRESS_CLAIMED, da, J1939_ECU_SA));
	RAMN_FDCAN_SendMessage(&header, j1939_name);
}

/* Send TP.CM BAM + TP.DT packets for ECU Identification (PGN 64965) */
static void J1939_SendEcuIdBAM(void)
{
	uint16_t totalSize = (uint16_t)(RAMN_strlen(j1939_ecu_id));
	uint8_t  numPackets = (uint8_t)((totalSize + 6U) / 7U);     /* ceiling division */
	FDCAN_TxHeaderTypeDef header;
	uint8_t payload[8];

	/* ---- TP.CM BAM ---- */
	J1939_InitTxHeader(&header, J1939_MakeId(7, J1939_PF_TP_CM, 0xFF, J1939_ECU_SA));
	payload[0] = J1939_TP_CM_BAM;
	payload[1] = (uint8_t)(totalSize & 0xFFU);
	payload[2] = (uint8_t)((totalSize >> 8) & 0xFFU);
	payload[3] = numPackets;
	payload[4] = 0xFFU;                                  /* reserved */
	payload[5] = (uint8_t)(J1939_PGN_ECU_ID & 0xFFU);
	payload[6] = (uint8_t)((J1939_PGN_ECU_ID >> 8) & 0xFFU);
	payload[7] = (uint8_t)((J1939_PGN_ECU_ID >> 16) & 0xFFU);
	RAMN_FDCAN_SendMessage(&header, payload);

	/* ---- TP.DT data packets ---- */
	header.Identifier = J1939_MakeId(7, J1939_PF_TP_DT, 0xFF, J1939_ECU_SA);
	for (uint8_t seq = 1; seq <= numPackets; seq++)
	{
		RAMN_memset(payload, 0xFF, 8U);
		payload[0] = seq;
		uint16_t offset = (uint16_t)((seq - 1U) * 7U);
		for (uint8_t j = 0; j < 7U && (offset + j) < totalSize; j++)
		{
			payload[1U + j] = (uint8_t)j1939_ecu_id[offset + j];
		}
		RAMN_FDCAN_SendMessage(&header, payload);
	}
}

/* Send TP Connection Abort back to the requestor */
static void J1939_SendTPConnAbort(uint8_t da, uint32_t pgn)
{
	FDCAN_TxHeaderTypeDef header;
	uint8_t payload[8];

	J1939_InitTxHeader(&header, J1939_MakeId(7, J1939_PF_TP_CM, da, J1939_ECU_SA));
	payload[0] = J1939_TP_CM_ABORT;
	payload[1] = 0xFF;
	payload[2] = 0xFF;
	payload[3] = 0xFF;
	payload[4] = 0xFF;
	payload[5] = (uint8_t)(pgn & 0xFFU);
	payload[6] = (uint8_t)((pgn >> 8) & 0xFFU);
	payload[7] = (uint8_t)((pgn >> 16) & 0xFFU);
	RAMN_FDCAN_SendMessage(&header, payload);
}

static void Pack_J1939_DTC(uint32_t uds_dtc, uint8_t* payload_ptr)
{
	uint32_t spn = 0;
	uint8_t fmi = 31;
	uint8_t oc = 1; // Hardcoded occurrence count

	uint16_t fault_code = (uds_dtc >> 16) & 0x3FFF;
	uint8_t category = (uds_dtc >> 30) & 0x03;

	if (category == 3 && fault_code == 0x0029) {
		// Bus A Performance (U0029)
		spn = 639; // J1939 Network #1
		fmi = 9;   // Abnormal update rate
	} else if (category == 1 && fault_code == 0x0563) {
		// Calibration ROM Checksum Error (C0563)
		spn = 628; // Program Memory
		fmi = 13;  // Out of Calibration
	} else if (category == 0 && fault_code == 0x0172) {
		// System too Rich (P0172)
		spn = 3055; // Engine Fuel System 1
		fmi = 0;    // Data valid but above normal
	} else if (category == 2 && fault_code == 0x0091) {
		// Active switch wrong state (B0091)
		spn = 2872; // Generic Switch
		fmi = 2;    // Data erratic, intermittent or incorrect
	} else {
		// Generic fallback
		spn = fault_code;
		fmi = 31;   // Condition exists
	}

	payload_ptr[0] = (uint8_t)(spn & 0xFF);
	payload_ptr[1] = (uint8_t)((spn >> 8) & 0xFF);
	payload_ptr[2] = (uint8_t)(((spn >> 16) & 0x07) << 5) | (fmi & 0x1F);
	payload_ptr[3] = (oc & 0x7F); // CM bit 7 = 0
}

static void J1939_SendNACK(uint8_t da, uint32_t pgn)
{
	FDCAN_TxHeaderTypeDef header;
	uint8_t payload[8];

	J1939_InitTxHeader(&header, J1939_MakeId(6, J1939_PF_ACK, da, J1939_ECU_SA));
	payload[0] = J1939_ACK_CONTROL_NACK;
	payload[1] = 0xFF; // Group Function Value
	payload[2] = 0xFF;
	payload[3] = 0xFF;
	payload[4] = 0xFF;
	payload[5] = (uint8_t)(pgn & 0xFFU);
	payload[6] = (uint8_t)((pgn >> 8) & 0xFFU);
	payload[7] = (uint8_t)((pgn >> 16) & 0xFFU);
	RAMN_FDCAN_SendMessage(&header, payload);
}

static void J1939_SendDM1(void)
{
	uint32_t numDTC = 0;
#ifdef ENABLE_EEPROM_EMULATION
	RAMN_DTC_GetNumberOfDTC(&numDTC);
#endif

	uint8_t mil = (RAMN_DBC_Handle.control_lights & 0x02) ? 1 : 0;
	uint8_t byte0 = (mil << 6) | 0x3F;
	uint8_t byte1 = 0xFF;

	if (numDTC <= 1)
	{
		FDCAN_TxHeaderTypeDef header;
		J1939_InitTxHeader(&header, J1939_MakeId(6, (J1939_PGN_DM1 >> 8) & 0xFF, J1939_PGN_DM1 & 0xFF, J1939_ECU_SA));

		uint8_t payload[8];
		RAMN_memset(payload, 0xFF, 8);
		payload[0] = byte0;
		payload[1] = byte1;

		if (numDTC == 1)
		{
			uint32_t dtc_val = 0;
#ifdef ENABLE_EEPROM_EMULATION
			RAMN_DTC_GetIndex(0, &dtc_val);
#endif
			Pack_J1939_DTC(dtc_val, &payload[2]);
		}
		else
		{
			payload[2] = 0x00;
			payload[3] = 0x00;
			payload[4] = 0x00;
			payload[5] = 0x00;
		}
		RAMN_FDCAN_SendMessage(&header, payload);
	}
	else
	{
		uint16_t totalSize = (uint16_t)(2 + (numDTC * 4));
		uint8_t  numPackets = (uint8_t)((totalSize + 6U) / 7U);
		FDCAN_TxHeaderTypeDef header;
		uint8_t payload[8];

		J1939_InitTxHeader(&header, J1939_MakeId(7, J1939_PF_TP_CM, 0xFF, J1939_ECU_SA));
		payload[0] = J1939_TP_CM_BAM;
		payload[1] = (uint8_t)(totalSize & 0xFFU);
		payload[2] = (uint8_t)((totalSize >> 8) & 0xFFU);
		payload[3] = numPackets;
		payload[4] = 0xFFU;
		payload[5] = (uint8_t)(J1939_PGN_DM1 & 0xFFU);
		payload[6] = (uint8_t)((J1939_PGN_DM1 >> 8) & 0xFFU);
		payload[7] = (uint8_t)((J1939_PGN_DM1 >> 16) & 0xFFU);
		RAMN_FDCAN_SendMessage(&header, payload);

		header.Identifier = J1939_MakeId(7, J1939_PF_TP_DT, 0xFF, J1939_ECU_SA);
		for (uint8_t seq = 1; seq <= numPackets; seq++)
		{
			RAMN_memset(payload, 0xFF, 8U);
			payload[0] = seq;
			uint16_t offset = (uint16_t)((seq - 1U) * 7U);
			for (uint8_t j = 0; j < 7U && (offset + j) < totalSize; j++)
			{
				uint8_t byte_val = 0xFF;
				uint16_t idx = offset + j;
				if (idx == 0) byte_val = byte0;
				else if (idx == 1) byte_val = byte1;
				else
				{
					uint32_t dtc_idx = (idx - 2) / 4;
					uint32_t dtc_byte = (idx - 2) % 4;
					uint32_t dtc_val = 0;
#ifdef ENABLE_EEPROM_EMULATION
					RAMN_DTC_GetIndex(dtc_idx, &dtc_val);
#endif
					uint8_t dtc_packed[4];
					Pack_J1939_DTC(dtc_val, dtc_packed);
					byte_val = dtc_packed[dtc_byte];
				}
				payload[1U + j] = byte_val;
			}
			RAMN_FDCAN_SendMessage(&header, payload);
		}
	}
}

void RAMN_J1939_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data)
{
	if (pHeader->IdType == FDCAN_EXTENDED_ID)
	{
		uint32_t canId  = pHeader->Identifier;
		uint8_t  pf     = J1939_GetPF(canId);
		uint8_t  ps_da  = J1939_GetPS(canId);
		uint8_t  dlc    = DLCtoUINT8(pHeader->DataLength);

		/* --- Handle Request PGN (PF = 0xEA) --- */
		if (pf == J1939_PF_REQUEST && dlc >= 3U)
		{
			/* Accept broadcast (DA=0xFF) or unicast to our SA */
			if (ps_da == J1939_DA_BROADCAST || ps_da == J1939_ECU_SA)
			{
				uint32_t reqPGN = (uint32_t)data[0]
				                | ((uint32_t)data[1] << 8)
				                | ((uint32_t)data[2] << 16);

				if (reqPGN == J1939_PGN_ADDRESS_CLAIMED)
				{
					J1939_SendAddressClaimed(ps_da == J1939_DA_BROADCAST ? 0xFF : J1939_GetSA(canId));
				}
				else if (reqPGN == J1939_PGN_ECU_ID)
				{
					J1939_SendEcuIdBAM();
				}
				else if (reqPGN == J1939_PGN_DM1)
				{
					J1939_SendDM1();
				}
				else if (ps_da == J1939_ECU_SA)
				{
					// For any unsupported PGN request directed specifically to us, send a NACK
					J1939_SendNACK(J1939_GetSA(canId), reqPGN);
				}
			}
		}

		/* --- Handle TP.CM RTS (PF = 0xEC, ctrl = 0x10) --- */
		if (pf == J1939_PF_TP_CM && ps_da == J1939_ECU_SA && dlc >= 8U)
		{
			if (data[0] == J1939_TP_CM_RTS)
			{
				uint32_t pgn = (uint32_t)data[5]
				             | ((uint32_t)data[6] << 8)
				             | ((uint32_t)data[7] << 16);
				J1939_SendTPConnAbort(J1939_GetSA(canId), pgn);
			}
		}
	}
}

#endif /* ENABLE_J1939_MODE */
