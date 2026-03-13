/*
 * ramn_customize.c
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

#include "ramn_customize.h"
#include "ramn_canfd.h"
#include "ramn_sensors.h"
#include "ramn_dbc.h"
#include "ramn_dtc.h"

#ifdef ENABLE_CDC
#include "ramn_cdc.h"
#endif

#ifdef ENABLE_UART
#include "ramn_uart.h"
#endif

// Loop counter for RAMN_CUSTOM_Update
static uint32_t loopCounter = 0;

// Number of time RAMN_CUSTOM_TIM6ISR has been called (by default, time in s from boot)
static volatile uint32_t tim6val = 0;

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
 * J1939 NAME (8 bytes, little-endian on the CAN bus).
 * Layout (64 bits, MSB first):
 *   Bit 63      : Arbitrary Address Capable = 0
 *   Bits 62-60  : Industry Group = 0
 *   Bits 59-56  : Vehicle System Instance = 0
 *   Bits 55-49  : Vehicle System = 0
 *   Bit 48      : Reserved = 0
 *   Bits 47-40  : Function (= primary SA for each ECU)
 *   Bits 39-35  : Function Instance = 0
 *   Bits 34-32  : ECU Instance = 0
 *   Bits 31-21  : Manufacturer Code = 0
 *   Bits 20-0   : Identity Number (unique per ECU: 1-4)
 */
static const uint8_t j1939_name[8] = {
#if defined(TARGET_ECUA)
	0x01, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x00  /* Identity=1, Function=42 (Headway Ctrl) */
#elif defined(TARGET_ECUB)
	0x02, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00  /* Identity=2, Function=19 (Steering Ctrl) */
#elif defined(TARGET_ECUC)
	0x03, 0x00, 0x00, 0x00, 0x00, 0x5A, 0x00, 0x00  /* Identity=3, Function=90 (Powertrain Ctrl) */
#elif defined(TARGET_ECUD)
	0x04, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00  /* Identity=4, Function=33 (Body Ctrl) */
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
	uint16_t totalSize = (uint16_t)(sizeof(j1939_ecu_id) - 1U); /* exclude null terminator */
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
		uint16_t totalSize = 2 + (numDTC * 4);
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

#endif /* ENABLE_J1939_MODE */

void 	RAMN_CUSTOM_Init(uint32_t tick)
{
	loopCounter = 0;
}

// Called when a CAN message is received (Hardware filters should be configured separately in ramn_canfd.c; with recvStdCANIDList and recvExtCANIDList)
// Note that by default, ECU A has no filter.
// This function is called from a task using an intermediary CAN buffer, so it does not need to return quickly.
void	RAMN_CUSTOM_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick)
{
	// Fields that you may want to use:
	// pHeader->Identifier: (11-bit val for standard, 29-bit for extended)
	// pHeader->IdType: FDCAN_STANDARD_ID or FDCAN_EXTENDED_ID
	// pHeader->RxFrameType:  FDCAN_DATA_FRAME or FDCAN_REMOTE_FRAME
	// DataLength: length of CAN payload, FDCAN_DLC_BYTES_0 (0) to FDCAN_DLC_BYTES_8 (8) for CAN, FDCAN_DLC_BYTES_0 (0) to FDCAN_DLC_BYTES_64 (0xF, Not 64) for CAN-FD.
	// pHeader->ErrorStateIndicator: For CAN-FD, either FDCAN_ESI_ACTIVE or FDCAN_ESI_PASSIVE
	// pHeader->BitRateSwitch: For CAN-FD, either FDCAN_BRS_OFF or FDCAN_BRS_ON
	// pHeader->FDFormat: FDCAN_CLASSIC_CAN or FDCAN_FD_CAN
	// pHeader->RxTimestamp: 16-bit value for RX timestamp, MAY NOT BE CONFIGURED CORRECTLY
	// See FilterIndex and IsFilterMatchingFrame for additional fields.

#ifdef ENABLE_J1939_MODE
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
#endif
}

#ifdef ENABLE_CDC
// This function is called when a USB serial (CDC) line is received (terminated by \r, which is not included in the buffer).
// if you need another type of line terminator, modify CDC_Receive_FS in usbd_cdc_if.c.
// Return True to ask the ECU to skip this line, return False to continue processing as usual.
// This function is called from a task using an intermediary USB buffer, so it does not need to return quickly.
RAMN_Bool_t RAMN_CUSTOM_ProcessCDCLine(uint8_t* buffer, uint32_t size)
{
	// If you return True, you can entirely override USB communications, meaning that ECU A will lose the ability to forward slcan commands.
	// This means that you will lose the ability to use RAMN scripts (including reflashing over USB DFU).
	// Only return True if that is the behavior that you expect, and have another method for ECU A reflashing.
	// If you want to make sure that you (at least) keep the option to reprogram ECU A, uncomment the line below and keep it at the beginning.
	// if (size > 0U && buffer[0] == 'D') return False;

	return False; // WARNING read comments above before editing
}
#endif

// Called periodically from main task (does not need to return quickly)
void RAMN_CUSTOM_Update(uint32_t tick)
{
	// This function is called by a dedicated periodic task, which means code can here won't block other functionalities (such as receiving CAN messages).
	// Modify SIM_LOOP_CLOCK_MS if you want to use another period than 10ms.

	// Code here is executed every 10ms

	if ((loopCounter % 10) == 0)
	{
		// Code here is executed every 100ms
	}

	if ((loopCounter % 100) == 0)
	{
		// Code here is executed every 1s

		// Example: send UART data every second
#ifdef ENABLE_UART
		RAMN_UART_SendStringFromTask("Hello from RAMN\r");
#endif

		// Example: Send a CAN message every second.
		// Note that if it is sent from ECU A, it will not show up on ECU A's USB interface (e.g., slcan), because ECU A will be the sender (and therefore not a receiver).
		/*
		FDCAN_TxHeaderTypeDef header;
		uint8_t data[8U];

		header.BitRateSwitch = FDCAN_BRS_OFF;			// Bitrate switching OFF (only needed for CAN-FD, but set anyway); other option is FDCAN_BRS_ON.
		header.ErrorStateIndicator = FDCAN_ESI_ACTIVE; 	// ESI bit (for CAN-FD only, but set anyway); other option is FDCAN_ESI_PASSIVE.
		header.FDFormat = FDCAN_CLASSIC_CAN; 			// Classic CAN; other option is FDCAN_FD_CAN.
		header.TxFrameType = FDCAN_DATA_FRAME;			// Data frame; other option is FDCAN_REMOTE_FRAME, only for classic CAN.
		header.IdType = FDCAN_STANDARD_ID;				// Standard identifier; other option is FDCAN_EXTENDED_ID for extended.
		header.Identifier = 0x123; 						// Identifier.
		header.DataLength = 8U;  						// DLC (Payload size).

		// Decide CAN message payload content
		RAMN_memset(data, 0x77, 8U); // write 0x77 8 times

		// Send message
		RAMN_FDCAN_SendMessage(&header,data);
		*/


		// Example: Execute every second, only if joystick is currently pressed down; only from ECU C (which is in charge of the sensor)
		// This is based on physical sensor data (ramn_sensors.h)
		/*
		if (RAMN_SENSORS_POWERTRAIN.shiftJoystick == RAMN_SHIFT_PUSH)
		{
			// Do something
		}
		*/

		// Example: Execute every second, only if joystick is currently pressed down; from ANOTHER ECU (other than ECU C)
		// This is based on the latest joystick CAN message received (ramn_dbc.h)
		// You need to make sure that the joystick CAN message is processed by adding #define RECEIVE_CONTROL_SHIFT in vehicle_specific.h
		/*
		if (RAMN_DBC_Handle.joystick == RAMN_SHIFT_PUSH)
		{
			// Do something
		}
		*/


	}

	loopCounter += 1; 	//You may want to add a check for integer overflow.
}

/* TIMERS */

// TIM16 is configured as a free running timer (e.g., to use for accurate timing measurements). Default: 1MHz counter (you can modify it without impacting other features).
// To reset TIM16 (e.g., to start a measurement), use:
// __HAL_TIM_SET_COUNTER(&htim16, 0);
// To read the value of TIM16 (to get your timing measurement), use:
// __HAL_TIM_GET_COUNTER(&htim16);  (should return uint16_t)

// TIM6  is configured as a trigger periodically calling the function below. Default: every 1s (you can modify it without impacting other features)

// Warning: This function is called within an ISR. It should not use freeRTOS functions not available to ISRs, and should return quickly.
void RAMN_CUSTOM_TIM6ISR(TIM_HandleTypeDef *htim)
{
	tim6val++;
}


/* TASK HOOKS */

// The functions below are called by tasks that are started but not used (e.g., USB task when USB is not active).
// They can be used to implement tasks that will not interfere with the main periodic task.
// Note that the priority of these tasks is typically higher than the periodic task, therefore they MUST periodically let other tasks execute (e.g. by calling vTaskDelayUntil or osDelay).
// Alternatively, if you want to execute slow and long code, you may alter the priority of the task.
// If you do not need these functions, use vTaskDelete(NULL) to delete the task.
// In all cases, make sure that you only modify the behavior of the targeted ECU, and not all ECUs (e.g., by using #ifdef TARGET_ECUB or #ifndef TARGET_ECUA).

#ifndef ENABLE_CDC
void RAMN_CUSTOM_CustomTask1(void *argument)
{
	//Called by RAMN_ReceiveUSBFunc
	vTaskDelete(NULL);
}

void RAMN_CUSTOM_CustomTask2(void *argument)
{
	//Called by RAMN_SendUSBFunc
	vTaskDelete(NULL);
}
#endif

#ifndef ENABLE_GSUSB
void RAMN_CUSTOM_CustomTask3(void *argument)
{
	//Called by RAMN_RxTask2Func
	vTaskDelete(NULL);
}

void RAMN_CUSTOM_CustomTask4(void *argument)
{
	//Called by RAMN_TxTask2Func
	vTaskDelete(NULL);
}
#endif

#ifndef ENABLE_DIAG
void RAMN_CUSTOM_CustomTask5(void *argument)
{
	//RAMN_DiagRXFunc
	vTaskDelete(NULL);
}

void RAMN_CUSTOM_CustomTask6(void *argument)
{
	//RAMN_DiagTXFunc
	vTaskDelete(NULL);
}
#endif


/* HARDWARE INTERFACE HOOKS */

#ifdef ENABLE_I2C
void RAMN_CUSTOM_ReceiveI2C(uint8_t buf[], uint16_t buf_size)
{
	// Warning: This function is called within an ISR. It should not use freeRTOS functions not available to ISRs, and should return quickly.
	// See RAMNV1.ioc for I2C device address (likely 0x77)
	// Note that by default, buf_size is fixed and equal to I2C_RX_BUFFER_SIZE. Function will NOT be called if fewer bytes are received.
	// You'll need to modify HAL_I2C_AddrCallback and HAL_I2C_SlaveRxCpltCallback in main.c if you need another behavior.
}

void RAMN_CUSTOM_PrepareTransmitDataI2C(uint8_t buf[], uint16_t buf_size)
{
	// Warning: This function is called within an ISR. It should not use freeRTOS functions not available to ISRs, and should return quickly.
	// Note that you cannot modify buf_size, only buf.
	// You'll need to modify HAL_I2C_AddrCallback in main.c if you need another behavior.
}
#endif

#ifdef ENABLE_UART

// You can send UART data using RAMN_UART_SendStringFromTask or RAMN_UART_SendFromTask, which are both non-blocking.
// This function is called from a task, with an intermediary UART buffer, and does not need to return quickly.
void RAMN_CUSTOM_ReceiveUART(uint8_t buf[], uint16_t buf_size)
{
	// By default, this function receives commands line by line, without endline character (\r)
	// You can modify this behavior in main.c (look for HAL_UART_Receive_IT and  HAL_UART_RxCpltCallback)
}
#endif
