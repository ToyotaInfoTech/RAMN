/*
 * ramn_uds.c
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

#include "ramn_uds.h"
#include <string.h>

#if defined(ENABLE_UDS)


// UDS Session Definitions
#define UDS_SESSION_DS	  0x01 // Default Session
#define UDS_SESSION_PRGS  0x02 // Programming Session
#define UDS_SESSION_EXTDS 0x03 // Extended Diagnostic Session
#define UDS_SESSION_SSDS  0x04 // Safety System Diagnostic Session

// UDS Reset Types
#define RESET_HARD_RESET    0x01
#define RESET_KEYOFFONRESET 0x02
#define RESET_SOFT_RESET    0x03
#define	RESET_ENABLERAPIDPOWERSHUTDOWN 0x04
#define RESET_DISABLERAPIDPOWERSHUTDOWN 0x05

typedef enum
{
	TRANSFER_IDLE = 0U,
	TRANSFER_DOWNLOADING,
	TRANSFER_UPLOADING,
	TRANSFER_FINISHED, // After last bit has been received/sent
	TRANSFER_VALIDATED,// After TransferExit has been called
} TransferManagerState_t;

struct {
	uint32_t startAddress;
	uint32_t size;
	uint32_t index;
	uint8_t seq;
	TransferManagerState_t status;
} transferManager;

struct {
	uint8_t newSettings;
} linkControlManager;

// Common pointer to avoid passing answer data as argument each sub-function
static uint8_t* uds_answerData;
static uint16_t* uds_answerSize;

typedef enum {
	SECURITY_ACCESS_UNAUTHENTICATED = 0,
	SECURITY_ACCESS_SEED_REQUESTED,
	SECURITY_ACCESS_SUCCESS
} securityAccessStatus_t;

typedef struct {
	uint32_t currentSeed;
	uint32_t currentKey;
	uint32_t lastAttemptTimestamp;
	uint32_t attemptNumber;
	securityAccessStatus_t status;
} securityAccessHandler_t;

struct {
	uint8_t currentSession;
	uint32_t lastMessageTimestamp;
	uint32_t lastTesterPresentTimestamp;
	securityAccessHandler_t defaultSAhandler;

} udsSessionHandler;


static FDCAN_TxHeaderTypeDef udsMsgHeader =
{
		.Identifier  = UDS_TX_CANID,
		.DataLength = FDCAN_DLC_BYTES_0,
		.IdType = FDCAN_STANDARD_ID,
		.TxFrameType = FDCAN_DATA_FRAME,
		.ErrorStateIndicator = FDCAN_ESI_ACTIVE,
		.BitRateSwitch = FDCAN_BRS_OFF,
		.FDFormat = FDCAN_CLASSIC_CAN,
		.TxEventFifoControl = FDCAN_NO_TX_EVENTS,
		. MessageMarker = 0,
};

static FDCAN_TxHeaderTypeDef udsFCMsgHeader =
{
		.Identifier  = UDS_TX_CANID,
		.DataLength = FDCAN_DLC_BYTES_0,
		.IdType = FDCAN_STANDARD_ID,
		.TxFrameType = FDCAN_DATA_FRAME,
		.ErrorStateIndicator = FDCAN_ESI_ACTIVE,
		.BitRateSwitch = FDCAN_BRS_OFF,
		.FDFormat = FDCAN_CLASSIC_CAN,
		.TxEventFifoControl = FDCAN_NO_TX_EVENTS,
		. MessageMarker = 0,
};

static void RAMN_UDS_FormatAnswer(const uint8_t* data, uint16_t size)
{
	if (size > 0)
	{
		for(uint16_t i = 0; i < size; i++)
		{
			uds_answerData[i] = data[i];
		}
	}
	*uds_answerSize = size;
}

// Formats a negative response based on provided error code
static void RAMN_UDS_FormatNegativeResponse(const uint8_t* data, uint8_t errCode)
{
	uds_answerData[0] = 0x7F;
	uds_answerData[1] = data[0];
	uds_answerData[2] = errCode;
	*uds_answerSize = 3U;
}

// Sends a positive response with provided size
static void RAMN_UDS_FormatPositiveResponseEcho(const uint8_t* data, uint16_t size)
{
	uds_answerData[0] = data[0] + 0x40;
	for(uint16_t i = 1; i < size; i++)
	{
		uds_answerData[i] = data[i];
	}
	*uds_answerSize = size;
}

// Check whether the current session is authenticated or not
static RAMN_Bool_t checkAuthenticated()
{
	return (udsSessionHandler.defaultSAhandler.status == SECURITY_ACCESS_SUCCESS);
}

// Resets the security handler
static void resetSAHandler(securityAccessHandler_t* sa)
{
	sa->currentSeed = 0;
	sa->currentKey = 0;
	sa->lastAttemptTimestamp = 0;
	sa->status = SECURITY_ACCESS_UNAUTHENTICATED;
	sa->attemptNumber = 0;
}

// This function check if session/authentication status allows for reprogramming
static uint8_t checkProgrammingOK(RAMN_Bool_t isSubfunction)
{
	uint8_t result = 0U;
	if (udsSessionHandler.currentSession != UDS_SESSION_PRGS)
	{
		result = isSubfunction ? UDS_NRC_SFNSIAS : UDS_NRC_SNSIAS;
	}
	else
	{
		if (checkAuthenticated() != True) result = UDS_NRC_SAD;
	}
	return result;
}

// Resets the UDS session
static void resetSession(uint32_t tick)
{
	udsSessionHandler.currentSession = UDS_SESSION_DS;
	transferManager.status = TRANSFER_IDLE;
	linkControlManager.newSettings = 0U;
	resetSAHandler(&(udsSessionHandler.defaultSAhandler));
	udsSessionHandler.lastMessageTimestamp = tick;
}

// UDS SERVICES IMPLEMENTATION ----------------------------------------------

static void RAMN_UDS_DiagnosticSessionControl(const uint8_t* data, uint16_t size)
{
	uint8_t answer[2] = {0x50, 0x00};

	if( size != 2U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		switch(data[1]&0x7F)
		{
		case UDS_SESSION_DS:
			resetSession(udsSessionHandler.lastMessageTimestamp);
			answer[1] = data[1];
			if ((data[1]&0x80) == 0) RAMN_UDS_FormatAnswer(answer, 2);
			break;
		case UDS_SESSION_EXTDS:
			// Check vehicle speed and only accept if stopped
			if (RAMN_DBC_Handle.status_rpm > UDS_MAXIMUM_RPM_ACCEPTABLE) RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_VSTH);
			else
			{
				resetSession(udsSessionHandler.lastMessageTimestamp);
				udsSessionHandler.currentSession = UDS_SESSION_EXTDS;
				answer[1] = data[1];
				if ((data[1]&0x80) == 0) RAMN_UDS_FormatAnswer(answer, 2);
			}
			break;
		case UDS_SESSION_PRGS:
			// Check vehicle speed and only accept if stopped
			if (RAMN_DBC_Handle.status_rpm > UDS_MAXIMUM_RPM_ACCEPTABLE) RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_CNC);
			else
			{
				resetSession(udsSessionHandler.lastMessageTimestamp);
				udsSessionHandler.currentSession = UDS_SESSION_PRGS;
				answer[1] = data[1];
				if ((data[1]&0x80) == 0) RAMN_UDS_FormatAnswer(answer, 2);
			}
			break;
		case UDS_SESSION_SSDS:
		default:
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
			break;
		}
	}
}

static void RAMN_UDS_ECUReset(const uint8_t* data, uint16_t size)
{
	if( size != 2U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else if (udsSessionHandler.currentSession == UDS_SESSION_DS)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNSIAS);
	}
	else
	{
		switch(data[1]&0x7F)
		{
		case RESET_HARD_RESET:
			RAMN_UDS_FormatPositiveResponseEcho(data, size);
			// Note that actual reset is performed after sending the answer, if necessary
			if ((data[1]&0x80) != 0U) HAL_NVIC_SystemReset(); //no need to answer, reset immediately
			break;
		default:
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
			break;
		}
	}
}

#ifndef HARDENING
static void RAMN_UDS_ClearDTC(const uint8_t* data, uint16_t size)
{
	// Only 0xFFFFFF (clear all) supported
	if (size != 4)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else if ((data[1U] != 0xFF) || (data[2U] != 0xFF) || (data[3U] != 0xFF))
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
	}
#ifdef ENABLE_EEPROM_EMULATION
	else if (RAMN_DTC_ClearAll() != 0U)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_GPF);
	}
#endif
	else
	{
		RAMN_UDS_FormatPositiveResponseEcho(data,1U);
	}
}

static void RAMN_UDS_ReadDTC(const uint8_t* data, uint16_t size)
{
#ifdef ENABLE_EEPROM_EMULATION
	uint32_t tmp;
	uint32_t numDTC;
	RAMN_Bool_t error = False;

	if (size < 3)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		uds_answerData[0] = 0x59;
		uds_answerData[1] = data[1];
		uds_answerData[2] = 0x04;
		switch(data[1]&0x7F)
		{
		case 0x01: // reportNumberOfDTCByStatusMask
			if (size != 3) RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
			else
			{
				// Masks not supported yet
				uds_answerData[3] = DTC_FORMAT_IDENTIFIER;
				if (RAMN_DTC_GetNumberOfDTC(&numDTC) != RAMN_OK)
				{
					RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_GPF);
				}
				else
				{
					uds_answerData[4] = (numDTC >> 8)&0xFF;
					uds_answerData[5] = (numDTC)&0xFF;
					RAMN_UDS_FormatAnswer(uds_answerData, 6);
				}
			}
			break;
		case 0x02: // reportDTCByStatusMask
			if (size != 3) RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
			else
			{
				// Masks not supported yet
				uds_answerData[3] = DTC_FORMAT_IDENTIFIER;
				if (RAMN_DTC_GetNumberOfDTC(&numDTC) != RAMN_OK)
				{
					RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_GPF);
				}
				else
				{
					for(uint32_t i = 0; i < numDTC; i++)
					{
						if (RAMN_DTC_GetIndex(i, &tmp) != RAMN_OK)
						{
							error = True;
						}
						else
						{
							uds_answerData[3+(4*i)] = (tmp >> 24)&0xFF;
							uds_answerData[4+(4*i)] = (tmp >> 16)&0xFF;
							uds_answerData[5+(4*i)] = (tmp >>  8)&0xFF;
							uds_answerData[6+(4*i)] = (tmp      )&0xFF;
						}
					}
					if (error == False)
					{
						RAMN_UDS_FormatAnswer(uds_answerData, 3 + 4*numDTC);
					}
					else
					{
						RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_GPF);
					}
				}
			}
			break;
		default:
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
			break;
		}
		if ((data[1]&0x80) != 0)
		{
			if (uds_answerData[0] != 0x7F) *uds_answerSize = 0U;
		}

	}
#else
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
#endif
}

// Note that bank alias are NOT accounted for (if bank swap is activated, the virtual addresses will be read)
static void RAMN_UDS_ReadMemoryByAddress(const uint8_t* data, uint16_t size)
{
	if( size < 2 )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else if ((data[1]&0x0F) != 0x04) // Only accept 32-bit addressing
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
	}
	else if ((data[1]&0xF0) > 0x40) // Only accept sizes below 32 bits
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
	}
	else if (udsSessionHandler.currentSession == UDS_SESSION_DS)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNSIAS);
	}
	else {

		uint32_t memsize = 0;
		uint8_t memsize_len = ((data[1]&0xF0)>> 4);

		if (size != 6 + memsize_len)
		{
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
		}
		else
		{
			uint32_t addr = (data[2] << 24) + (data[3] << 16) + (data[4] << 8) + (data[5]);

			for(uint8_t i = 0; i < memsize_len; i++)
			{
				memsize |= data[6+i] << ((memsize_len - (i+1))*8);
			}
			if (memsize > 0xFFE)
			{
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
			}
			else if ((RAMN_MEMORY_CheckAreaReadable(addr, (addr + (uint32_t)memsize)) != 0U) && (memsize < 0xFFF))
			{
				uds_answerData[0] = data[0] + 0x40; // Positive response
				for(uint32_t i = 0; i < memsize; i++ )
				{
					uds_answerData[i+1] = (volatile uint8_t)*((volatile uint8_t*)(addr+i));
				}
				*uds_answerSize = (uint16_t)memsize+1;
			}
#if defined(ENABLE_MINICTF) && defined(TARGET_ECUD)
			else if ((addr == 0x01234567) && memsize <= (sizeof(FLAG_UDS_4)-1))
			{
				uds_answerData[0] = data[0] + 0x40; // Positive response
				for(uint32_t i = 0; i < memsize; i++ )
				{
					uds_answerData[i+1] = (volatile uint8_t)*((volatile uint8_t*)(FLAG_UDS_4+i));
				}
				*uds_answerSize = (uint16_t)memsize+1;
			}
#endif
			else
			{
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
			}
		}
	}
}
#endif // HARDENING


static void RAMN_UDS_ReadDataByIdentifier(uint8_t* data, uint16_t size)
{
#if defined(ENABLE_EEPROM_EMULATION)
	if( size != 3U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		uint16_t index = (data[1] << 8) + data[2];
		uint32_t val[5] = {0};
		uint8_t answer[32+4];
		uint8_t answer_size = 0;
		EE_Status result = EE_OK;
#ifdef ENABLE_EEPROM_EMULATION
		if ((index >= 0x200) && (index < 0x400))
		{
			// This range can be read/written with arbitrary 32-bit values
			result = RAMN_EEPROM_Read32(index,val);
			if (result == EE_OK)
			{
				answer[0] = data[0] + 0x40;
				answer[1] = data[1];
				answer[2] = data[2];
				RAMN_memcpy((uint8_t*)&(answer[3]),(uint8_t*)val,4);
				RAMN_UDS_FormatAnswer(answer, 7);
			}
			else if (result != EE_OK)
			{
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
			}
		}
		else
#endif
		{
			switch (index)
			{
#if defined(ENABLE_MINICTF) && defined(TARGET_ECUD)
			case 0x0001:
				RAMN_memcpy(&(answer[3]),(const uint8_t*)FLAG_UDS_1,sizeof(FLAG_UDS_1)-1);
				answer_size = 3+(sizeof(FLAG_UDS_1)-1);
				break;
			case 0x0002:
				if (checkAuthenticated() == True)
				{
					RAMN_memcpy(&(answer[3]),(const uint8_t*)FLAG_UDS_5,sizeof(FLAG_UDS_5)-1);
					answer_size = 3+(sizeof(FLAG_UDS_5)-1);
				}
				else
				{
					data[0] = 0x3F; // Will become 0x7F
					data[1] = 0x22;
					data[2] = UDS_NRC_SAD;
					answer_size = 3;
				}
				break;
			case 0x7742:
				RAMN_memcpy(&(answer[3]),(const uint8_t*)FLAG_UDS_2,sizeof(FLAG_UDS_2)-1);
				answer_size = 3+(sizeof(FLAG_UDS_2)-1);
				break;
#endif
			case 0xF184: // Compile Date and Time
				RAMN_memcpy(&(answer[3]),(uint8_t*)__DATE__,sizeof(__DATE__));
				answer[3+sizeof(__DATE__)] = ' ';
				RAMN_memcpy(&(answer[3+1+sizeof(__DATE__)]),(uint8_t*)__TIME__,sizeof(__TIME__));
				answer_size = 3 + 1 + sizeof(__DATE__) + sizeof(__TIME__);
				break;
			case 0xF186: // Active Session
				answer[3] = udsSessionHandler.currentSession;
				answer_size = 1+3;
				break;
			case 0xF187: // Spare part
				RAMN_memcpy(&(answer[3]),(uint8_t*)"RAMN",4U);
				answer_size = 4+3;
				break;
			case 0xF18C: // ECU Serial Hardware
				RAMN_memcpy((uint8_t*)&(answer[3]),(uint8_t*)DEVICE_HARDWARE_ID_ADDRESS,12);
				answer_size = 12+3;
				break;
#ifdef ENABLE_EEPROM_EMULATION
			case 0xF190: // VIN
				result |= RAMN_EEPROM_Read32(VIN_BYTES1_4_INDEX,val);
				result |= RAMN_EEPROM_Read32(VIN_BYTES5_8_INDEX,&(val[1]));
				result |= RAMN_EEPROM_Read32(VIN_BYTES9_12_INDEX,&(val[2]));
				result |= RAMN_EEPROM_Read32(VIN_BYTES13_16_INDEX,&(val[3]));
				result |= RAMN_EEPROM_Read32(VIN_BYTES17_20_INDEX,&(val[4]));
				RAMN_memcpy((uint8_t*)&(answer[3]),(uint8_t*)val,17);
				answer_size = 17+3;
				break;
#endif
			default:
				result = EE_INVALID_VIRTUALADDRESS; //not a positive response
				break;
			}
			if (result == EE_OK)
			{
				answer[0] = data[0] + 0x40;
				answer[1] = data[1];
				answer[2] = data[2];
				RAMN_UDS_FormatAnswer(answer, answer_size);
			}
			else
			{
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
			}
		}
	}
#else
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
#endif
}

#ifndef HARDENING

static void RAMN_UDS_ReadScalingDataByIdentifier(const uint8_t* data, uint16_t size)
{
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
}

static void RAMN_UDS_RequestSeed(const uint8_t* data, uint16_t size)
{
	uint8_t answer[6] = {0x67, data[1]&0x7F, 0x00, 0x00, 0x00, 0x00};
	udsSessionHandler.defaultSAhandler.currentSeed = RAMN_RNG_Pop32();
	udsSessionHandler.defaultSAhandler.currentKey  = udsSessionHandler.defaultSAhandler.currentSeed ^ 0x12345678;
	answer[2] = (uint8_t)(udsSessionHandler.defaultSAhandler.currentSeed >> 24)&0xFF;
	answer[3] = (uint8_t)(udsSessionHandler.defaultSAhandler.currentSeed >> 16)&0xFF;
	answer[4] = (uint8_t)(udsSessionHandler.defaultSAhandler.currentSeed >> 8 )&0xFF;
	answer[5] = (uint8_t)(udsSessionHandler.defaultSAhandler.currentSeed      )&0xFF;
	udsSessionHandler.defaultSAhandler.status = SECURITY_ACCESS_SEED_REQUESTED;
	if ((data[1]&0x80) == 0U) RAMN_UDS_FormatAnswer(answer, 6);
}

static void RAMN_UDS_TryKey(const uint8_t* data, uint16_t size)
{
	uint8_t answer[2] = {0x67, data[1]&0x7F};
	uint32_t try = (data[2] << 24) + (data[3] << 16) + (data[4] << 8) + (data[5]);

	if(udsSessionHandler.defaultSAhandler.attemptNumber >= SECURITY_ACCESS_MAX_ATTEMPTS)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ENOA);
	}
	else if(udsSessionHandler.lastMessageTimestamp - udsSessionHandler.defaultSAhandler.lastAttemptTimestamp < SECURITY_ACCESS_RETRY_TIMEOUT_MS)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_RTDNE);
	}

	else if (udsSessionHandler.defaultSAhandler.status != SECURITY_ACCESS_SEED_REQUESTED)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_RSE);
	}
	else if (try == udsSessionHandler.defaultSAhandler.currentKey)
	{
		udsSessionHandler.defaultSAhandler.status = SECURITY_ACCESS_SUCCESS;
		if ((data[1]&0x80) == 0U) RAMN_UDS_FormatAnswer(answer, 2);
	}
	else
	{
		udsSessionHandler.defaultSAhandler.status =  SECURITY_ACCESS_UNAUTHENTICATED;
		udsSessionHandler.defaultSAhandler.attemptNumber++;
		udsSessionHandler.defaultSAhandler.lastAttemptTimestamp = udsSessionHandler.lastMessageTimestamp;
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IK);
	}
}

static void RAMN_UDS_SecurityAccess(const uint8_t* data, uint16_t size)
{
	if( size <  2U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		switch(data[1]&0x7F)
		{
		case 0x01:
			RAMN_UDS_RequestSeed(data, size);
			break;
		case 0x02:
			if( size !=  6U ) RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
			else RAMN_UDS_TryKey(data, size);
			break;
		default:
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
			break;
		}
	}
}


static void RAMN_UDS_CommunicationControl(const uint8_t* data, uint16_t size)
{
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
}

static void RAMN_UDS_Authentication(const uint8_t* data, uint16_t size)
{
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
}

static void RAMN_UDS_ReadDataByIdentifierPeriodic(const uint8_t* data, uint16_t size)
{
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
}

static void RAMN_UDS_DynamicallyDefineDataIdentifier(const uint8_t* data, uint16_t size)
{
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
}

static void RAMN_UDS_WriteDataByIdentifier(const uint8_t* data, uint16_t size)
{
#if defined(ENABLE_EEPROM_EMULATION)
	if( size < 3U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		uint16_t index = (data[1] << 8) + data[2];
		EE_Status result = EE_OK;

		if (udsSessionHandler.currentSession != UDS_SESSION_PRGS )
		{
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNSIAS);
		}
		else
		{
			if ( (index == 0xF184) || (index == 0xF18C))
			{
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
			}
			else if (((index != 0xF190) && (size != 7U)) || ((index == 0xF190) && (size != 20)))
			{
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
			}
			else
			{
				if (index != 0xF190) // NOT VIN
				{
					uint32_t val = (data[3] << 24) + (data[4] << 16) + (data[5] << 8) + (data[6]);
					result = RAMN_EEPROM_Write32(index,val);
				}
				else // VIN
				{
					result |= RAMN_EEPROM_Write32(VIN_BYTES1_4_INDEX,(data[6] << 24) + (data[5] << 16) + (data[4] << 8) + (data[3]));
					result |= RAMN_EEPROM_Write32(VIN_BYTES5_8_INDEX,(data[10] << 24) + (data[9] << 16) + (data[8] << 8) + (data[7]));
					result |= RAMN_EEPROM_Write32(VIN_BYTES9_12_INDEX,(data[14] << 24) + (data[13] << 16) + (data[12] << 8) + (data[11]));
					result |= RAMN_EEPROM_Write32(VIN_BYTES13_16_INDEX,(data[18] << 24) + (data[17] << 16) + (data[16] << 8) + (data[15]));
					result |= RAMN_EEPROM_Write32(VIN_BYTES17_20_INDEX,(data[19]));
				}

				if (result != EE_OK)
				{
					RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_GPF);
				}
				else
				{
					uint8_t answer[3];
					answer[0] = data[0]+0x40;
					answer[1] = data[1];
					answer[2] = data[2];
					RAMN_UDS_FormatAnswer(answer, 3);
				}
			}
		}
	}
#else
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
#endif
}

static void RAMN_UDS_InputOutputControlByIdentifier(const uint8_t* data, uint16_t size)
{
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
}

#endif //HARDENING

// Routine Control to ask the ECU to erase the alternative firmware
static void RAMN_UDS_RoutineControlEraseAlternativeFirmware(const uint8_t* data, uint16_t size)
{
	uint8_t errCode;
	if( size != 4U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else {
		switch (data[1]){
		case 0x01:// Start
			errCode = checkProgrammingOK(True);
			if (errCode == 0U)
			{
				if (RAMN_FLASH_EraseAlternativeFirmware() != RAMN_OK) RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_GPF);
				else RAMN_UDS_FormatPositiveResponseEcho(data, size);
			}
			else
			{
				RAMN_UDS_FormatNegativeResponse(data, errCode);
			}
			break;
		case 0x02:// Stop
		case 0x03:// Read Results
		default: // Invalid
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
			break;
		}
	}
}

// Routine control to request the ECU to validate a firmware update
static void RAMN_UDS_RoutineControlValidateMemory(const uint8_t* data, uint16_t size)
{
	uint8_t errCode;
	if( size != 4U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else{
		switch (data[1]){
		case 0x01:// Start
			errCode = checkProgrammingOK(True);
			if (errCode == 0U)
			{
				if(transferManager.status == TRANSFER_VALIDATED)
				{
					if (RAMN_FLASH_SwitchActiveBank() != RAMN_OK)
					{
						RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_GPF);
					}
					else
					{
						RAMN_UDS_FormatPositiveResponseEcho(data, size);
					}
				}
				else
				{
					RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_CNC);
				}
			}
			else
			{
				RAMN_UDS_FormatNegativeResponse(data, errCode);
			}
			break;
		case 0x02:// Stop
		case 0x03:// Read Results
		default: // Invalid
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
			break;
		}
	}
}

//Routine to ask the ECU to stop/start sending periodic messages
static void RAMN_UDS_RoutineControlRequestSilence(const uint8_t* data, uint16_t size)
{
	if( size != 4U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else{
		switch (data[1]){
		case 0x01:// Start
			RAMN_DBC_RequestSilence = True;
			RAMN_UDS_FormatPositiveResponseEcho(data, size);
			break;
		case 0x02:// Stop
			RAMN_DBC_RequestSilence = False;
			RAMN_UDS_FormatPositiveResponseEcho(data, size);
			break;
		case 0x03:// Read Results
		default: // Invalid
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
			break;
		}
	}
}

// Emergency Routine to reset BOOT0 option bytes
// Used to salvage a board on which the wrong firmware was flashed, that refuses to go to bootloader mode
// Note that this command should reset board automatically, so it should not answer
static void RAMN_UDS_RoutineControlResetBootOptionBytes(const uint8_t* data, uint16_t size)
{
	if( size != 4U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else{

		uint8_t errCode = checkProgrammingOK(False);
		if (errCode != 0U)
		{
			RAMN_UDS_FormatNegativeResponse(data, errCode);
		}
		else
		{
			RAMN_Result_t result = RAMN_OK;
			switch (data[1]){
			case 0x01:// Start
				if(RAMN_FLASH_isMemoryProtected() == False)
				{
					result = RAMN_FLASH_ConfigureOptionBytesBootloaderMode();
				}
				else
				{
					result = RAMN_FLASH_RemoveMemoryProtection();
				}
				if (result != RAMN_OK)
				{
					RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_GPF);
				}
				else
				{
					RAMN_UDS_FormatPositiveResponseEcho(data, size);
				}
				break;
			default: // Invalid
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
				break;
			}
		}
	}
}

// Emergency Routine to force an ECU to swap its memory banks.
// This can be used to revert to a previous firmware (WITHOUT ANY CHECK) and fix issues when STM32 embedded bootloader/STM32CubeProgrammer/STM32CubeIDE do not work well with bank swaps.
// Do not use unless you know what you are doing.
static void RAMN_UDS_RoutineControlForceMemorySwap(const uint8_t* data, uint16_t size)
{
	if( size != 4U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else{
		uint8_t errCode = checkProgrammingOK(False);
		if (errCode != 0U)
		{
			RAMN_UDS_FormatNegativeResponse(data, errCode);
		}
		else
		{
			switch (data[1]){
			case 0x01:// Start
				if (RAMN_FLASH_SwitchActiveBank() != RAMN_OK)
				{
					RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_GPF);
				}
				else
				{
					RAMN_UDS_FormatPositiveResponseEcho(data, size);
				}
				break;
			default: // Invalid
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
				break;
			}
		}
	}
}


// Routine Control that returns the CRC for specified area
static void RAMN_UDS_RoutineControlComputeCRC(const uint8_t* data, uint16_t size)
{
	if (size < 12)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		uint32_t startaddr = (data[4] << 24) + (data[5] << 16) + (data[6] << 8) + (data[7]);
		uint32_t memsize = (data[8] << 24) + (data[9] << 16) + (data[10] << 8) + (data[11]);
		uint32_t endaddr = startaddr + memsize;
		if  (RAMN_MEMORY_CheckAreaReadable(startaddr,endaddr) == False)
		{
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
		}
		else
		{
			uint32_t result = RAMN_CRC_SoftCalculate((uint8_t*)startaddr,memsize);
			uds_answerData[0] = data[0] + 0x40; // Positive response
			uds_answerData[1] = data[1];
			uds_answerData[2] = data[2];
			uds_answerData[3] = data[3];
			uds_answerData[4] = (uint8_t)(result >> 24)&0xFF;
			uds_answerData[5] = (uint8_t)(result >> 16)&0xFF;
			uds_answerData[6] = (uint8_t)(result >> 8 )&0xFF;
			uds_answerData[7] = (uint8_t)(result      )&0xFF;
			*uds_answerSize = 8U;
		}
	}
}


//Routine Control to enable or disable autopilot features
#if defined(TARGET_ECUB) || defined(TARGET_ECUC) || defined(TARGET_ECUD)

static void RAMN_UDS_RoutineControlAutopilot(const uint8_t* data, uint16_t size)
{
	if( size != 4U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else{
		switch (data[1]){
		case 0x01:// Start
			RAMN_SIM_AutopilotEnabled = True;
			RAMN_UDS_FormatPositiveResponseEcho(data, size);
			break;
		case 0x02:// Stop
			RAMN_SIM_AutopilotEnabled = False;
			RAMN_UDS_FormatPositiveResponseEcho(data, size);
			break;
		case 0x03:// Read Results
		default: // Invalid
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
			break;
		}
	}
}
#endif

#ifndef HARDENING
#ifdef ENABLE_EEPROM_EMULATION
// Routine to add an arbitrary DTC entry
static void RAMN_UDS_RoutineControlAddDTCEntry(const uint8_t* data, uint16_t size)
{
	if( size != 8U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else{
		switch (data[1]){
		case 0x01:		// Start
			if (data[7] != 0x04)
			{
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
			}
			else
			{
				uint32_t dtc_val = (data[4] << 24U) | (data[5] << 16U) | (data[6] << 8U) | (data[7]);
				if (RAMN_DTC_AddNew(dtc_val) == RAMN_OK)
				{
					RAMN_UDS_FormatPositiveResponseEcho(data, 4U);
				}
				else
				{
					RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_GPF);
				}
			}
			break;
		case 0x02:  	// Stop
		case 0x03: 		// Read Results
		default:  	 	// Invalid
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
			break;
		}
	}
}
#endif

#ifdef ENABLE_SCREEN

#ifdef ENABLE_CHIP8
static void loadChip8Game(const uint8_t* data, uint16_t size)
{
	uint16_t game_size = 0;
	if(size < 3U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		game_size = (uint16_t)(data[1] << 8) | data[2];
		if (game_size != (size-3))
		{
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
		}
		else if (game_size > (0x1000 - 0x250) || game_size < 2)
		{
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
		}
		else
		{
			RAMN_CHIP8_StopGame();
			osDelay(200); // Leave some time to quit the game.
			RAMN_SCREENMANAGER_RequestGame(&data[3], game_size);
			uds_answerData[0] = data[0] + 0x40; // Positive response
			*uds_answerSize = 1;
		}
	}
}
#endif

static void displayPixels(const uint8_t* data, uint16_t size)
{
	uint8_t x = 0;
	uint8_t y = 0;
	uint32_t w = 0;
	uint32_t h = 0;
	if(size < 5U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else {
		x = data[1] % 240;
		y = data[2] % 240;
		w = data[3];
		h = data[4];
		if ((w*h)*2 != (size-5))
		{
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
		}
		else if ((w == 0 || h == 0) || ((w*h) > 57600))
		{
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
		}
		else
		{
			if (RAMN_SCREENMANAGER_IsUDSScreenUpdatePending() == 0U)
			{
				RAMN_ScreenManager_RequestDrawImage(x,y,w,h,&data[5]);
				uds_answerData[0] = data[0] + 0x40; //positive response
				*uds_answerSize = 1;
			}
			else
			{
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_BRR); //busy, ask to repeat later
			}
		}
	}
}
#endif
#endif // HARDENING


#ifndef HARDENING // Double-check to make sure these do not end up in hardened code.

typedef void (*exec_func_t)(void);
// Routine to execute arbitrary code (e.g., to test ARM shellcode)
static void RAMN_UDS_RoutineControlExecuteArbitraryCode(const uint8_t* data, uint16_t size)
{
	if (checkAuthenticated() == True)
	{
		exec_func_t func = (exec_func_t)(uintptr_t)(&data[4] + 1); //use +1 to force thumb mode, data must be aligned.
		__DSB();
		__ISB();
		func();
		RAMN_UDS_FormatPositiveResponseEcho(data, 4U);
	}
	else RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SAD);
}

#ifdef ENABLE_MINICTF
const char expected_password[] = "VULNERABILITY";

// Routine to demonstrate simple overflow vulnerability, variable in stack
static void RAMN_UDS_RoutineControlVulnerabilityExample(uint8_t* data, uint16_t size)
{
	if (checkAuthenticated() == True)
	{
		char stack_password[sizeof(expected_password)];

		if ((size > 4U) && (size < ISOTP_RXBUFFER_SIZE))
		{
			data[size] = 0U; //zero-terminate buffer
			strcpy(stack_password, (char*)&data[4U]); //copy string
			if (strcmp(stack_password, expected_password) == 0U) RAMN_UDS_FormatPositiveResponseEcho(data, 4U);
			else RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
		}
		else RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SAD);
}

// Routine to demonstrate simple overflow vulnerability, variable in .bss
char ctf_global_password[sizeof(expected_password)];
static void RAMN_UDS_RoutineControlVulnerabilityExample2(uint8_t* data, uint16_t size)
{
	if (checkAuthenticated() == True)
	{
		if ((size > 4U) && (size < ISOTP_RXBUFFER_SIZE))
		{
			data[size] = 0U; //zero-terminate buffer
			strcpy(ctf_global_password, (char*)&data[4U]); //copy string
			if (strcmp(ctf_global_password, expected_password) == 0U) RAMN_UDS_FormatPositiveResponseEcho(data, 4U);
			else RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
		}
		else RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SAD);
}

#endif
#endif

// 0000 to 00FF ISO Reserved
// 0100 to 01FF Tachograph
// 0200 to 0DFF Manufacturer Specific
// 0E00 to E1FF OBD Test IDs
// E200 Deploy Loop Routine ID
// E201 to E2FF Safety System Routine IDs
// E300 to EFFF ISO Reserved
// F000 to FEFF System Supplier Specific
// FF00 Erase Memory
// FF01 Check Programming Dependencies
// FF02 Erase Mirror DTCs
// FF03 to FFFF ISO Reserved

// 0200 To Disable Periodic Sending of messages
// 0201 To Erase the EEPROM
// 0202 To Copy current values in EEPROM to alternative EEPROM
static void RAMN_UDS_RoutineControl(uint8_t* data, uint16_t size)
{
	if( size < 4U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		uint8_t supprPositiveResponse = 0U;
		if ((data[1]&0x80) != 0)
		{
			supprPositiveResponse = 1U;
			data[1] = data[1]&0x7F;
		}
		switch( ((data[2] << 8 ) | data[3])){ // subroutine (2 bytes)
#ifdef ENABLE_UDS_REPROGRAMMING
		case 0xFF00: // Erase Memory
			RAMN_UDS_RoutineControlEraseAlternativeFirmware(data, size);
			break;
		case 0xFF01: // Validate Memory and Swap Banks
			RAMN_UDS_RoutineControlValidateMemory(data, size);
			break;
#endif
		case 0x0200: // Request Silence
			RAMN_UDS_RoutineControlRequestSilence(data, size);
			break;
#ifdef ENABLE_UDS_REPROGRAMMING
		case 0x0201: // Erase EEPROM
			if (RAMN_FLASH_EraseActiveEEPROM() == RAMN_OK) RAMN_UDS_FormatPositiveResponseEcho(data, size);
			else RAMN_UDS_FormatNegativeResponse(data,UDS_NRC_GPF);
			break;
		case 0x0202: // Copy Current EEPROM to alternative firmware:
			if (RAMN_FLASH_CopyEEPROMToInactiveBank() == RAMN_OK) RAMN_UDS_FormatPositiveResponseEcho(data, size);
			else RAMN_UDS_FormatNegativeResponse(data,UDS_NRC_GPF);
			break;
#endif
		case 0x0203: // Echo Message (full Message)
			RAMN_UDS_FormatPositiveResponseEcho(data,size);
			break;
		case 0x0204: // Echo Message (4 bytes only)
			RAMN_UDS_FormatPositiveResponseEcho(data,4U);
			break;
		case 0x0205: // Fixed Echo with specified size (Full Buffer)
			if (size == 6U)
			{
				uint16_t memsize = (uint16_t)(data[4] << 8) + data[5];
				if (memsize > 0xFFB) {
					RAMN_UDS_FormatNegativeResponse(data,UDS_NRC_ROOR);
				}
				else
				{
					uds_answerData[0] = data[0]+0x40;
					uds_answerData[1] = data[1];
					uds_answerData[2] = data[2];
					uds_answerData[3] = data[3];
					for (uint16_t i = 0; i < memsize-4;i++)
					{
						uds_answerData[4+i] = (uint8_t)(i&0xFF);
					}
					*uds_answerSize = memsize+4;
				}
			}
			else RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
			break;
#ifdef ENABLE_UDS_REPROGRAMMING
		case 0x0206: // Compute CRC
			RAMN_UDS_RoutineControlComputeCRC(data,size);
			break;
#endif
#if defined(TARGET_ECUB) || defined(TARGET_ECUC) || defined(TARGET_ECUD)
		case 0x0207: // Enable/Disable Autopilot features:
			RAMN_UDS_RoutineControlAutopilot(data,size);
			break;
#endif
#ifndef HARDENING
#if defined(ENABLE_EEPROM_EMULATION)
		case 0x0208: // Add DTC Entry:
			RAMN_UDS_RoutineControlAddDTCEntry(data,size);
			break;
#endif
		case 0x0209: // Execute Arbitrary code
			RAMN_UDS_RoutineControlExecuteArbitraryCode(data,size);
			break;
#ifdef ENABLE_MINICTF
		case 0x020A: // Vulnerable password check
			RAMN_UDS_RoutineControlVulnerabilityExample(data,size);
			break;
		case 0x020B: // Vulnerable password check
			RAMN_UDS_RoutineControlVulnerabilityExample2(data,size);
			break;
#endif
#endif // HARDENING
#ifdef ENABLE_UDS_REPROGRAMMING
		case 0x0210: // Reset Option Bytes:
			RAMN_UDS_RoutineControlResetBootOptionBytes(data,size);
			break;
		case 0x0211: // Force Memory Swap:
			RAMN_UDS_RoutineControlForceMemorySwap(data,size);
			break;
#endif
		default:
			RAMN_UDS_FormatNegativeResponse(data,UDS_NRC_ROOR);
			break;
		}
		if ((*uds_answerSize > 0) && (supprPositiveResponse != 0U))
		{
			if (uds_answerData[0] != 0x7F) *uds_answerSize = 0U;
		}
	}
}

static void RAMN_UDS_RequestDownloadUpload(const uint8_t* data, uint16_t size, TransferManagerState_t direction)
{
	uint8_t errCode;
	if( size < 3U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else if ((data[1] != 0x00) || (data[2] != 0x44))
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
	}
	else if( size != 11U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		errCode = checkProgrammingOK(False);
		if (errCode == 0U)
		{
			if((transferManager.status == TRANSFER_IDLE) || (transferManager.status == TRANSFER_VALIDATED))
			{
				transferManager.startAddress = (data[3] << 24) + (data[4] << 16) + (data[5] << 8) + (data[6]);
				transferManager.index = 0;
				transferManager.seq = 1;
				transferManager.size = (data[7] << 24) + (data[8] << 16) + (data[9] << 8) + (data[10]);
				if (RAMN_FLASH_CheckFlashAreaValidForFirmware(transferManager.startAddress,transferManager.startAddress+transferManager.size) != 0U)
				{
					if ((transferManager.size % 8) != 0)
					{
						// Only writeable in 64 bytes by bytes
						RAMN_UDS_FormatNegativeResponse(data,UDS_NRC_UDNA);
						// TODO: pad internally ?
					}
					uint8_t answer[4];
					answer[0] = data[0] + 0x40;
					answer[1] = 0x20;
					answer[2] = (TRANSFER_DATA_BLOCKSIZE >> 8)&0xFF;
					answer[3] = (TRANSFER_DATA_BLOCKSIZE)&0xFF;
					transferManager.status = direction;
					RAMN_UDS_FormatAnswer(answer, sizeof(answer));
				}
				else
				{
					RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
				}
			}
			else
			{
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_RSE);
			}
		}
		else
		{
			RAMN_UDS_FormatNegativeResponse(data, errCode);
		}
	}
}

static void RAMN_UDS_RequestDownload(const uint8_t* data, uint16_t size)
{
#if defined(ENABLE_UDS_REPROGRAMMING)
	uint16_t memsize = *(const uint16_t*)FLASHSIZE_BASE;
	if (memsize == 512)
	{
		RAMN_UDS_RequestDownloadUpload(data, size,TRANSFER_DOWNLOADING);
	}
	else
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
	}
#else
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
#endif
}

#ifndef HARDENING
static void RAMN_UDS_RequestUpload(const uint8_t* data, uint16_t size)
{
	RAMN_UDS_RequestDownloadUpload(data, size,TRANSFER_UPLOADING);
}
#endif

static void RAMN_UDS_TransferData(const uint8_t* data, uint16_t size)
{
#if defined(ENABLE_UDS_REPROGRAMMING)
	if( size < 2U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		if ((transferManager.status == TRANSFER_DOWNLOADING))
		{
			uint16_t payloadSize = size -2;
			RAMN_Result_t result = RAMN_OK;
			// Memory must be 64-bit aligned, without overflowing
			if (((payloadSize % 8) == 0U) && ((transferManager.index + payloadSize) <= transferManager.size))
			{
				if (data[1] == transferManager.seq)
				{
					transferManager.seq = (transferManager.seq +1)&0xFF;
					HAL_FLASH_Unlock();
					for(uint32_t i = 0U; i < payloadSize; i+=8U)
					{
						uint32_t msb = (data[i+9] << 24) + (data[i+8] << 16) + (data[i+7] << 8) + (data[i+6]);
						uint32_t lsb = (data[i+5] << 24) + (data[i+4] << 16) + (data[i+3] << 8) + (data[i+2]);
						uint64_t data =  ((uint64_t)msb << 32) + lsb;
						result |= RAMN_FLASH_Write64(FLASH_FIRMWARE_BANK_OFFSET+ transferManager.startAddress + transferManager.index,data);
						transferManager.index += 8U;
					}
					HAL_FLASH_Lock();

					if(transferManager.index >= transferManager.size)
					{
						// Download succeeded
						transferManager.status = TRANSFER_FINISHED;
					}

					if(result == RAMN_OK)
					{
						uint8_t answer[2];
						answer[0] = data[0] + 0x40;
						answer[1] = data[1];
						RAMN_UDS_FormatAnswer(answer, sizeof(answer));
					}
					else
					{
						RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_GPF);
					}
				}
				else
				{
					RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_WBSC);
				}
			}
			else
			{
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_TDS);
			}

		}
		else if ((transferManager.status == TRANSFER_UPLOADING))
		{
			if (size == 2U)
			{
				if (data[1] == transferManager.seq)
				{
					transferManager.seq = (transferManager.seq +1)&0xFF;
					uint16_t count = 0;
					// RAMN_memcpy to buffer to send
					uds_answerData[0] = data[0] + 0x40;
					uds_answerData[1] = data[1];
					for(uint16_t i = 0; (i <TRANSFER_DATA_BLOCKSIZE) && (transferManager.index < transferManager.size);i++)
					{
						uds_answerData[2+i] = (uint8_t)*((uint8_t*)(transferManager.startAddress+transferManager.index));
						transferManager.index++;
						count++;

					}
					*uds_answerSize = count+2;

					if(transferManager.index >= transferManager.size)
					{
						// Download succeeded
						transferManager.status = TRANSFER_FINISHED;
					}
				}
				else
				{
					RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_WBSC);
				}
			}
			else
			{
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_TDS);
			}
		}
		else
		{
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_RSE);
		}
	}
#else
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
#endif
}

static void RAMN_UDS_RequestTransferExit(const uint8_t* data, uint16_t size)
{
#if defined(ENABLE_UDS_REPROGRAMMING)
	if( size != 1U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		if (transferManager.status == TRANSFER_FINISHED)
		{
			transferManager.status = TRANSFER_VALIDATED;
			RAMN_UDS_FormatPositiveResponseEcho(data, size);
		}
		else
		{
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_RSE);
		}
	}
#else
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
#endif
}

#ifndef HARDENING
static void RAMN_UDS_RequestFileTransfer(const uint8_t* data, uint16_t size)
{
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
}

static void RAMN_UDS_WriteMemoryByAddress(const uint8_t* data, uint16_t size)
{
	if( size < 2U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else if ((data[1]&0x0F) != 0x04) // Only accept 32-bit addressing
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
	}
	else if ((data[1]&0xF0) > 0x40) // Only accept sizes below 32 bits
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
	}
	else
	{
		uint16_t writeSize = 0;
		uint8_t memsize_len = ((data[1]&0xF0)>> 4);

		if (size < 6 + memsize_len)
		{
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
		}

		else
		{
			for(uint8_t i = 0; i < memsize_len; i++)
			{
				writeSize |= data[6+i] << ((memsize_len - (i+1))*8);
			}

			if (size != (6 + memsize_len + writeSize))
			{
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
			}
			else
			{
				uint32_t startaddr = (data[2] << 24) + (data[3] << 16) + (data[4] << 8) + (data[5]);
				uint8_t errCode;

				errCode = checkProgrammingOK(False);
				if (errCode != 0U)
				{
					RAMN_UDS_FormatNegativeResponse(data, errCode);
				}
				else
				{
					if (RAMN_RAM_CheckAreaWritable(startaddr,startaddr+(uint32_t)writeSize) == False)
					{
						RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
					}
					else
					{
						if (writeSize == 2U) // Write short all at once
						{
							*(volatile uint16_t*)(startaddr) = (uint16_t)(((uint16_t)data[8]<<8) + (uint16_t)data[9]);
						}
						else if (writeSize == 4U) // Write integer all at once
						{
							*(volatile uint32_t*)(startaddr) = (uint32_t)(((uint32_t)data[8] << 24) + ((uint32_t)data[9] << 16) + ((uint32_t)data[10] << 8) + ((uint32_t)data[11]));
						}
						else if (writeSize == 8U) // Write long all at once
						{
							*(volatile uint64_t*)(startaddr) = (uint64_t)(((uint64_t)data[8] << 56) + ((uint64_t)data[9] << 48) + ((uint64_t)data[10] << 40) + ((uint64_t)data[11] << 32) + ((uint64_t)data[12] << 24) + ((uint64_t)data[13] << 16) + ((uint64_t)data[14] << 8) + ((uint64_t)data[15]));
						}
						else // Write byte by byte
						{
							for (uint32_t i = 0U; i < (uint32_t)writeSize; i++)
								*(volatile uint8_t*)(startaddr) = (uint8_t)data[8+i];
						}
						RAMN_UDS_FormatPositiveResponseEcho(data,6+memsize_len);
					}
				}
			}
		}
	}
}
#endif

static void RAMN_UDS_TesterPresent(const uint8_t* data, uint16_t size)
{
	if( size != 2U )
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		switch(data[1])
		{
		case 0x00:
			RAMN_UDS_FormatPositiveResponseEcho(data, size);
			break;
		case 0x80:
			// No answer, as requested
			break;
		default:
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
			break;
		}
	}
}

#ifndef HARDENING
static void RAMN_UDS_AccessTimingParameters(const uint8_t* data, uint16_t size)
{
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
}

static void RAMN_UDS_SecuredDataTransmission(const uint8_t* data, uint16_t size)
{
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
}

static void RAMN_UDS_ControlDTCSettings(const uint8_t* data, uint16_t size)
{
	if ( size != 2U)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else
	{
		if ((data[1]&0x7F) == 0x1U)
		{
#ifdef EEPROM_EMULATION
			RAMN_DTC_SetRecordingStatus(1U);
#endif
			if ((data[1]&0x80) == 0U) RAMN_UDS_FormatPositiveResponseEcho(data, size);
		}
		else if ((data[1]&0x7F) == 0x02U)
		{
#ifdef EEPROM_EMULATION
			RAMN_DTC_SetRecordingStatus(0U);
#endif
			if ((data[1]&0x80) == 0U) RAMN_UDS_FormatPositiveResponseEcho(data, size);
		}
		else
		{
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
		}
	}
}

static void RAMN_UDS_ResponseOnEvent(const uint8_t* data, uint16_t size)
{
	RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
}
#endif

static void performLinkControlUpdate()
{
	// Request Silence
	uint8_t tmp = RAMN_DBC_RequestSilence;
	RAMN_DBC_RequestSilence = 1U;
	RAMN_FDCAN_Disable();
	// Delay to let other ECU adapt to new baudrate
	osDelay(1000);
	// Authorize communication again
	RAMN_FDCAN_UpdateBaudrate(linkControlManager.newSettings);
	RAMN_FDCAN_ResetPeripheral();
	// Restore original settings
	RAMN_DBC_RequestSilence = tmp;
}

static void RAMN_UDS_LinkControl(const uint8_t* data, uint16_t size)
{
	if (size <= 1U) {
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
		return;
	}
	switch (data[1]&0x7F) {
	case 0x01:
		if (size < 3U) {
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
			return;
		}
		switch (data[2]) {
		case 0x10: // 125000
			linkControlManager.newSettings = '4';
			break;
		case 0x11: // 250000
			linkControlManager.newSettings = '5';
			break;
		case 0x12: // 500000
			linkControlManager.newSettings = '6';
			break;
		case 0x13: // 1000000
			linkControlManager.newSettings = '8';
			break;
		case 0x01: // 9600
		case 0x02: // 19200
		case 0x03: // 38400
		case 0x04: // 57600
		case 0x05: // 115200
		case 0x20: // ProgrammingSetup
		default:
			linkControlManager.newSettings = 0U;
			break;
		}
		if (linkControlManager.newSettings != 0U) {
			if ((data[1] & 0x80) == 0U) {
				// Send response if required
				RAMN_UDS_FormatPositiveResponseEcho(data, 2U);
			}
		} else {
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
		}
		break;
		case 0x83: // suppressPosRspMsgIndicationBit
		case 0x03:
			if (size < 2U) {
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
				return;
			}
			if (linkControlManager.newSettings == 0U) {
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_CNC);
			} else {

				if ((data[1] & 0x80) == 0U) {
					// Send response if required
					RAMN_UDS_FormatPositiveResponseEcho(data, 2U);
					// Actual update performed after answer is sent
				}
				else
				{
					// If no answer requested, perform action now
					performLinkControlUpdate();
				}
			}
			break;
		default:
			RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
			break;

	}
}

#if defined(ENABLE_MINICTF) && defined(TARGET_ECUD)
static void processCustomServiceFlag(const uint8_t* data, uint16_t size)
{
	if (size!= 4U)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
	else if (data[1] != 0x77)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SFNS);
	}
	else if (udsSessionHandler.currentSession == UDS_SESSION_DS)
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNSIAS);
	}
	else if ((data[2] == 0x13) && data[3] == 0x37)
	{
		uds_answerData[0] = data[0] + 0x40;
		RAMN_memcpy(&uds_answerData[1], (const uint8_t*)FLAG_UDS_3, sizeof(FLAG_UDS_3)-1);
		*uds_answerSize = sizeof(FLAG_UDS_3);
	}
	else
	{
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_ROOR);
	}
}

#endif
/* END OF UDS IMPLEMENTATION */

// Exported Components ---------------------------------------

__attribute__ ((section (".buffers"))) RAMN_ISOTPHandler_t RAMN_UDS_ISOTPHandler;

RAMN_Result_t RAMN_UDS_Init(uint32_t tick)
{
	resetSession(tick);
	RAMN_ISOTP_Init(&RAMN_UDS_ISOTPHandler,&udsFCMsgHeader);
	return RAMN_OK;
}

RAMN_Result_t RAMN_UDS_Update(uint32_t tick)
{
	// Only timeout for extended diagnostics, not for programming (?)
	if (udsSessionHandler.currentSession == UDS_SESSION_EXTDS)
	{
		if ((tick - udsSessionHandler.lastMessageTimestamp) > UDS_SESSION_TIMEOUT_MS) resetSession(tick);
	}
	return RAMN_ISOTP_Update(&RAMN_UDS_ISOTPHandler,tick);
}

RAMN_Bool_t RAMN_UDS_Continue_TX(uint32_t tick)
{
	return RAMN_ISOTP_Continue_TX(&RAMN_UDS_ISOTPHandler, tick, &udsMsgHeader);
}

RAMN_Bool_t RAMN_UDS_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick, StreamBufferHandle_t* strbuf)
{
	size_t xBytesSent;
	uint8_t functionalAddressing = 0U;
	RAMN_Bool_t result = False;
	uint16_t requestSize = 0;
	if (pHeader->Identifier == UDS_RX_CANID)
	{
		RAMN_ISOTP_ProcessRxMsg(&RAMN_UDS_ISOTPHandler,DLCtoUINT8(pHeader->DataLength),data, tick);

		// If a ISO-TP has been received, copy it to buffer
		if (RAMN_UDS_ISOTPHandler.rxStatus == ISOTP_RX_FINISHED)
		{
			functionalAddressing = 0U;
			xBytesSent = xStreamBufferSend(*strbuf, (void *) &(functionalAddressing), sizeof(functionalAddressing), portMAX_DELAY );
			xBytesSent += xStreamBufferSend(*strbuf, (void *) &(RAMN_UDS_ISOTPHandler.rxCount), sizeof(RAMN_UDS_ISOTPHandler.rxCount), portMAX_DELAY );
			xBytesSent += xStreamBufferSend(*strbuf, (void *) RAMN_UDS_ISOTPHandler.rxData, RAMN_UDS_ISOTPHandler.rxCount, portMAX_DELAY );
			if( xBytesSent != (sizeof(functionalAddressing) + RAMN_UDS_ISOTPHandler.rxCount + sizeof(RAMN_UDS_ISOTPHandler.rxCount))) Error_Handler();
			RAMN_UDS_ISOTPHandler.rxStatus = ISOTP_RX_IDLE;
			result = True;
		}
	}
#ifdef UDS_ACCEPT_FUNCTIONAL_ADDRESSING
	else if(pHeader->Identifier == UDS_FUNCTIONAL_RX_CANID)
	{
		if (DLCtoUINT8(pHeader->DataLength) > 0)
		{
			if(((data[0]&0xF0) >> 4) == 0x0U) // SingleFrame
			{
				if (((data[0]&0xF) > 0) && ((data[0]&0xF) <= 7U) && ((data[0]&0xF) <= (DLCtoUINT8(pHeader->DataLength)-1U)))
				{
					// Valid frame
					functionalAddressing = 1U;
					requestSize = data[0]&0xF;
					xBytesSent = xStreamBufferSend(*strbuf, (void *) &(functionalAddressing), sizeof(functionalAddressing), portMAX_DELAY );
					xBytesSent += xStreamBufferSend(*strbuf, (void *) &(requestSize), sizeof(requestSize), portMAX_DELAY );
					xBytesSent += xStreamBufferSend(*strbuf, (void *) &data[1], requestSize, portMAX_DELAY );
					if( xBytesSent != (sizeof(functionalAddressing) + requestSize + sizeof(RAMN_UDS_ISOTPHandler.rxCount))) Error_Handler();
					result = True;
				}
			}
		}
	}
#endif

	return result;
}

void RAMN_UDS_ProcessDiagPayload(uint32_t tick, uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize)
{
	uds_answerData = answerData;
	uds_answerSize = answerSize;

	*uds_answerSize = 0U; // Empty Response by default

	udsSessionHandler.lastMessageTimestamp = tick;
	if (size > 0U)
	{
		if (data[0] < 0x10)
		{
			// J1979 command
			RAMN_J1979_ProcessMessage(data, size, answerData, answerSize);
		}
		else // ISO14229 command
		{
			switch (data[0]) {
			case 0x10: // DIAGNOSTIC SESSION CONTROL:
				RAMN_UDS_DiagnosticSessionControl(data, size);
				break;
			case 0x11: // ECU RESET
				RAMN_UDS_ECUReset(data, size);
				break;
#ifndef HARDENING
			case 0x14: // CLEAR DIAGNOSTIC INFORMATION
				RAMN_UDS_ClearDTC(data, size);
				break;
			case 0x19: // READ DTC INFORMATION
				RAMN_UDS_ReadDTC(data, size);
				break;
#endif
			case 0x22: // READ DATA BY IDENTIFIER
				RAMN_UDS_ReadDataByIdentifier(data, size);
				break;
#ifndef HARDENING
			case 0x23: // READ MEMORY BY ADDRESS
				RAMN_UDS_ReadMemoryByAddress(data, size);
				break;
			case 0x24: // READ SCALING DATA BY IDENTIFIER
				RAMN_UDS_ReadScalingDataByIdentifier(data, size);
				break;
			case 0x27: // SECURITY ACCESS
				RAMN_UDS_SecurityAccess(data, size);
				break;
			case 0x28: // COMMUNICATION CONTROL
				RAMN_UDS_CommunicationControl(data, size);
				break;
			case 0x29: // AUTHENTICATION
				RAMN_UDS_Authentication(data, size);
				break;
			case 0x2A: // READ DATA BY IDENTIFIER PERIODIC
				RAMN_UDS_ReadDataByIdentifierPeriodic(data, size);
				break;
			case 0x2C: // DYNAMICALLY DEFINE DATA IDENTIFIER
				RAMN_UDS_DynamicallyDefineDataIdentifier(data, size);
				break;
			case 0x2E: // WRITE DATA BY IDENTIFIER
				RAMN_UDS_WriteDataByIdentifier(data, size);
				break;
			case 0x2F: // INPUT OUTPUT CONTROL BY IDENTIFIER
				RAMN_UDS_InputOutputControlByIdentifier(data, size);
				break;
#endif
			case 0x31:
				RAMN_UDS_RoutineControl(data, size);
				break;
			case 0x34: // REQUEST DOWNLOAD
				RAMN_UDS_RequestDownload(data, size);
				break;
#ifndef HARDENING
			case 0x35: // REQUEST UPLOAD
				RAMN_UDS_RequestUpload(data, size);
				break;
#endif
			case 0x36: // TRANSFER DATA
				RAMN_UDS_TransferData(data, size);
				break;
			case 0x37: // REQUEST TRANSFER EXIT
				RAMN_UDS_RequestTransferExit(data, size);
				break;
#ifndef HARDENING
			case 0x38: // REQUEST FILE TRANSFER
				RAMN_UDS_RequestFileTransfer(data, size);
				break;
			case 0x3D: // WRITE MEMORY BY ADDRESS
				RAMN_UDS_WriteMemoryByAddress(data, size);
				break;
#endif
			case 0x3E: // TESTER PRESENT
				RAMN_UDS_TesterPresent(data, size);
				break;
#if defined(ENABLE_SCREEN) && !defined(HARDENING)
			case 0x41: // Custom service to display pixels on screen
				displayPixels(data, size);
				break;
#if defined(ENABLE_CHIP8)
			case 0x42: // Custom service to load chip-8 games
				loadChip8Game(data, size);
				break;
#endif
#endif
#if defined(ENABLE_MINICTF) && defined(TARGET_ECUD)
			case 0x40:
				processCustomServiceFlag(data,size);
				break;
#endif
#ifndef HARDENING
			case 0x83: // ACCESS TIMING PARAMETERS
				RAMN_UDS_AccessTimingParameters(data, size);
				break;
			case 0x84: // SECURED DATA TRANSMISSION
				RAMN_UDS_SecuredDataTransmission(data, size);
				break;
			case 0x85: // CONTROL DTC SETTINGS
				RAMN_UDS_ControlDTCSettings(data, size);
				break;
			case 0x86: // RESPONSE ON EVENT
				RAMN_UDS_ResponseOnEvent(data, size);
				break;
#endif
			case 0x87: // LINK CONTROL
				RAMN_UDS_LinkControl(data, size);
				break;
			default:  // UNSUPPORTED SERVICES
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
				break;
			}
		}
	}
	else
	{
		data[0] = 0U;
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
}


void RAMN_UDS_ProcessDiagPayloadFunctional(uint32_t tick, uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize)
{
	uds_answerData = answerData;
	uds_answerSize = answerSize;

	*uds_answerSize = 0U; // Empty Response by default

	udsSessionHandler.lastMessageTimestamp = tick;
	if (size > 0U)
	{
		if (data[0] < 0x10)
		{
			// J1979 command
			RAMN_J1979_ProcessMessage(data, size, answerData, answerSize);
		}
		else // ISO14229 command
		{
			switch (data[0]) {
			case 0x10: // DIAGNOSTIC SESSION CONTROL:
				RAMN_UDS_DiagnosticSessionControl(data, size);
				break;
			case 0x11: // ECU RESET
				RAMN_UDS_ECUReset(data, size);
				break;
#ifndef HARDENING
			case 0x14: // CLEAR DIAGNOSTIC INFORMATION
				RAMN_UDS_ClearDTC(data, size);
				break;
			case 0x27: // SECURITY ACCESS
				RAMN_UDS_SecurityAccess(data, size);
				break;
#endif
			case 0x3E: // TESTER PRESENT
				RAMN_UDS_TesterPresent(data, size);
				break;
#ifndef HARDENING
			case 0x85: // CONTROL DTC SETTINGS
				RAMN_UDS_ControlDTCSettings(data, size);
				break;
#endif
			case 0x87: // LINK CONTROL
				RAMN_UDS_LinkControl(data, size);
				break;
			default:  // UNSUPPORTED SERVICES
				RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_SNS);
				break;
			}
		}
	}
	else
	{
		data[0] = 0U;
		RAMN_UDS_FormatNegativeResponse(data, UDS_NRC_IMLOIF);
	}
}

void RAMN_UDS_PerformPostAnswerActions(uint32_t tick, const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize)
{
	if (*answerSize > 0){
		switch (answerData[0])
		{
		case 0x51:
			// Reset was accepted
			osDelay(500);
			HAL_NVIC_SystemReset();
			break;
		case 0xC7:
			// Link Control update was accepted
			if (*answerSize > 1)
			{
				if (answerData[1] == 0x03)
				{
					osDelay(500);
					performLinkControlUpdate();
				}
			}
			break;
		default:
			break;

		}
	}
}

#endif
