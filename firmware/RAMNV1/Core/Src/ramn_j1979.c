/*
 * ramn_j1979.c
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

#include "ramn_j1979.h"
#include <string.h>

#if defined(ENABLE_UDS) || defined(ENABLE_KWP)

#ifdef ENABLE_J1979

static void formatNegativeResponse(const uint8_t* data, uint8_t* answerData, uint16_t* answerSize, uint8_t errCode)
{
	answerData[0] = 0x7F;
	answerData[1] = data[0];
	answerData[2] = errCode;
	*answerSize = 3U;
}

static void showCurrentData(const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize)
{
	if (size != 2U) formatNegativeResponse(data, answerData, answerSize, J1979_NRC_SFNSIF);
	else
	{
		// Valid format
		uint32_t numDTC;

		answerData[0] = data[0] + 0x40;
		answerData[1] = data[1];
		switch(data[1U]) // PID value
		{
		case 0x00:
			// PIDs supported (01 to 0x20)
			*answerSize = 6U;
#ifdef TARGET_ECUC
			answerData[2U] = 0xBE;
			answerData[3U] = 0x1F;
			answerData[4U] = 0xA8;
			answerData[5U] = 0x13;
#else
			answerData[2U] = 0x98;
			answerData[3U] = 0x18;
			answerData[4U] = 0x80;
			answerData[5U] = 0x03;
#endif
			break;
		case 0x01:
			// Monitor Status since DTCs cleared
#ifdef ENABLE_EEPROM_EMULATION
			// Read DTCs
			if (RAMN_DTC_GetNumberOfDTC(&numDTC) != RAMN_OK) numDTC = 0;
#else
			numDTC = 0U;
#endif
			*answerSize = 6U;
			answerData[2U] = numDTC&0x7F; // By default Consider MIL off and all DTCs emission related
			answerData[3U] = 0x00;
			answerData[4U] = 0x00;
			answerData[5U] = 0x00;
			break;
#ifdef TARGET_ECUC
		case 0x03:
			// Fuel System Status
			*answerSize = 4U;
			answerData[2U] = 0x02; // Closed-loop
			answerData[3U] = 0x00;
			break;
#endif
		case 0x04:
			// Calculated engine load
			*answerSize = 3U;
			answerData[2U] = 0x4C;
			break;
		case 0x05:
			// Engine coolant temperature
			*answerSize = 3U;
			answerData[2U] = 0x74;
			break;
#ifdef TARGET_ECUC
		case 0x06:
			// Short term fuel trim
			*answerSize = 3U;
			answerData[2U] = 0x7E;
			break;
		case 0x07:
			// Long term fuel trim
			*answerSize = 3U;
			answerData[2U] = 0x88;
			break;
#endif
		case 0x0C:
			// Engine Speed (in rpm, must be divided by 4).
			// We multiply by 8 to reach about 8000 rpm at max accelerator.
			*answerSize = 4U;
			answerData[2U] = (uint8_t)(((RAMN_DBC_Handle.control_accel*8) >> 8)&0xFF);
			answerData[3U] = (uint8_t)((RAMN_DBC_Handle.control_accel*8) & 0xFF);
			break;
		case 0x0D:
			// Vehicle Speed (in km/h). Only available with CARLA, units may need to be updated.
			*answerSize = 3U;
			answerData[2U] = (uint8_t)((RAMN_DBC_Handle.status_rpm >> 8)&0xFF);
			break;
#ifdef TARGET_ECUC
		case 0x0E:
			// Timing advance
			*answerSize = 3U;
			answerData[2U] = 0x8A;
			break;
		case 0x0f:
			// Intake air temperature
			*answerSize = 3U;
			answerData[2U] = 0x8A;
			break;
#endif
#ifdef TARGET_ECUC
		case 0x10:
			// Massive air flow sensor air flow rate
			*answerSize = 4U;
			answerData[2U] = 0x00;
			answerData[3U] = 0x27;
			break;
#endif
		case 0x11:
			// Throttle position
			*answerSize = 3U;
			answerData[2U] = 0x00;
			break;
#ifdef TARGET_ECUC
		case 0x13:
			// Oxygen sensors present
			*answerSize = 3U;
			answerData[2U] = 0x03;
			break;
		case 0x15:
			// Oxygen sensor 2
			*answerSize = 4U;
			answerData[2U] = 0x00;
			answerData[3U] = 0xFF;
			break;
		case 0x1C:
			// OBD standard
			*answerSize = 3U;
			answerData[2U] = 0x0A; // JOBD
			break;
#endif
		case 0x1F:
			// Run time since engine start (in s)
			*answerSize = 4U;
			answerData[2U] = ((xTaskGetTickCount()/1000) >> 8U)&0xFF;
			answerData[3U] = (xTaskGetTickCount()/1000)&0xFF;
			break;
		case 0x20:
			// PIDS supported (0x21 to 0x40)
			*answerSize = 6U;
#ifdef TARGET_ECUC
			answerData[2U] = 0x80;
			answerData[3U] = 0x05;
			answerData[4U] = 0xB0;
			answerData[5U] = 0x15;
#else
			answerData[2U] = 0x80;
			answerData[3U] = 0x01;
			answerData[4U] = 0x80;
			answerData[5U] = 0x01;
#endif
			break;
		case 0x21:
			// Distance traveled with MIL on
			*answerSize = 4U;
			answerData[2U] = 0x00;
			answerData[3U] = 0x00;
			break;
#ifdef TARGET_ECUC
		case 0x2e:
			// commanded evaporative purge
			*answerSize = 3U;
			answerData[2U] = 0x00;
			break;
#endif
		case 0x30:
			// Warm-ups since codes cleared
			*answerSize = 3U;
			answerData[2U] = 0x00;
			break;
		case 0x31:
			// Distance traveled since codes cleared
			*answerSize = 4U;
			answerData[2U] = 0x00;
			answerData[3U] = 0x00;
			break;
#ifdef TARGET_ECUC
		case 0x33:
			// Absolute Barometric Pressure
			*answerSize = 3U;
			answerData[2U] = 0x64;
			break;
		case 0x34:
			// Oxygen Sensor 1
			*answerSize = 6U;
			answerData[2U] = 0x8c;
			answerData[3U] = 0x92;
			answerData[4U] = 0x80;
			answerData[5U] = 0x23;
			break;
		case 0x3C:
			// Catalyst Temperature
			*answerSize = 4U;
			answerData[2U] = 0x13;
			answerData[3U] = 0x8A;
			break;
		case 0x3E:
			// Catalyst Temperature
			*answerSize = 4U;
			answerData[2U] = 0x0b;
			answerData[3U] = 0x39;
			break;
#endif
		case 0x40:
			// PIDs supported (0x41 to 0x60)
			*answerSize = 6U;
#ifdef TARGET_ECUC
			answerData[2U] = 0x7E;
			answerData[3U] = 0xDC;
			answerData[4U] = 0x8C;
			answerData[5U] = 0x01;
#elif defined(TARGET_ECUD)
			answerData[2U] = 0xA4;
			answerData[3U] = 0x0C;
			answerData[4U] = 0x00;
			answerData[5U] = 0x21;
#else
			answerData[2U] = 0xA4;
			answerData[3U] = 0x0C;
			answerData[4U] = 0x00;
			answerData[5U] = 0x01;
#endif
			break;
#ifndef TARGET_ECUC
		case 0x41:
			// Monitor status this drive cycle
			*answerSize = 6U;
			answerData[2U] = 0x00;
			answerData[3U] = 0x00;
			answerData[4U] = 0x00;
			answerData[5U] = 0x00;
			break;
#endif
		case 0x42:
			// Control module voltage
			*answerSize = 4U;
			answerData[2U] = 0x35;
			answerData[3U] = 0xBF;
			break;
#ifdef TARGET_ECUC
		case 0x43:
			// Absolute load value
			*answerSize = 4U;
			answerData[2U] = 0x00;
			answerData[3U] = 0x00;
			break;
		case 0x44:
			// Commanded Air-Fuel Equivalence
			*answerSize = 4U;
			answerData[2U] = 0x80;
			answerData[3U] = 0x13;
			break;
		case 0x45:
			// Relative throttle position
			*answerSize = 3U;
			answerData[2U] = 0x00;
			break;
#endif
		case 0x46:
			// Ambient air temperature
			*answerSize = 3U;
			answerData[2U] = 0x51;
			break;
#ifdef TARGET_ECUC
		case 0x47:
			// Absolute throttle position B
			*answerSize = 3U;
			answerData[2U] = 0x00;
			break;
		case 0x49:
			// Accelerator pedal position
			*answerSize = 3U;
			answerData[2U] = (RAMN_SENSORS_POWERTRAIN.accelPotentiometer >> 4)&0xFF;
			break;
		case 0x4a:
			// Accelerator pedal position E
			*answerSize = 3U;
			answerData[2U] = (RAMN_SENSORS_POWERTRAIN.accelPotentiometer >> 4)&0xFF;
			break;
		case 0x4c:
			// Commanded throttle actuator
			*answerSize = 3U;
			answerData[2U] = (RAMN_SENSORS_POWERTRAIN.accelPotentiometer >> 4)&0xFF;
			break;
#endif
		case 0x4d:
			// Time run with MIL on
			*answerSize = 4U;
			answerData[2U] = 0x00;
			answerData[3U] = 0x00;
			break;
		case 0x4E:
			// Time since trouble codes cleared
			*answerSize = 4U;
			answerData[2U] = 0x00;
			answerData[3U] = 0x00;
			break;
#ifdef TARGET_ECUC
		case 0x51:
			// Fuel Type
			*answerSize = 3U;
			answerData[2U] = 0x01;
			break;
		case 0x55:
			// Short term secondary oxygen
			*answerSize = 3U;
			answerData[2U] = 0x80;
			break;
		case 0x56:
			// Long term secondary oxygen
			*answerSize = 3U;
			answerData[2U] = 0x80;
			break;
#endif
#ifdef TARGET_ECUD
		case 0x5B:
			// Hybrid battery pack remaining life
			*answerSize = 3U;
			answerData[2U] = 0x6D;
			break;
#endif
		case 0x60:
			// PIDs supported (0x61 to 0x80)
			*answerSize = 6U;
#ifdef TARGET_ECUC
			answerData[2U] = 0x60;
			answerData[3U] = 0x80;
			answerData[4U] = 0x00;
			answerData[5U] = 0x01;
#else
			answerData[2U] = 0x00;
			answerData[3U] = 0x00;
			answerData[4U] = 0x00;
			answerData[5U] = 0x01;
#endif
			break;
#ifdef TARGET_ECUC
		case 0x62:
			// Actual engine - percent torque
			*answerSize = 3U;
			answerData[2U] = 0x7D;
			break;
		case 0x63:
			// Engine reference torque
			*answerSize = 4U;
			answerData[2U] = 0x00;
			answerData[3U] = 0x93;
			break;
		case 0x69:
			// Actual EGR, command EGR, EGR error
			*answerSize = 9U;
			answerData[2U] = 0x69;
			answerData[3U] = 0x01;
			answerData[4U] = 0x00;
			answerData[5U] = 0x00;
			answerData[6U] = 0x00;
			answerData[7U] = 0x00;
			answerData[8U] = 0x00;
			break;
#endif
		case 0x80:
			// PIDs supported (0x81 to 0xA0)
			*answerSize = 6U;
#ifdef TARGET_ECUC
			answerData[2U] = 0x02;
			answerData[3U] = 0x04;
			answerData[4U] = 0x00;
			answerData[5U] = 0x0C;
#elif defined(TARGET_ECUB)
			answerData[2U] = 0x00;
			answerData[3U] = 0x00;
			answerData[4U] = 0x00;
			answerData[5U] = 0x01;
#elif defined(TARGET_ECUA)
			answerData[2U] = 0x00;
			answerData[3U] = 0x00;
			answerData[4U] = 0x00;
			answerData[5U] = 0x00;
#else // ECU D
			answerData[2U] = 0x00;
			answerData[3U] = 0x00;
			answerData[4U] = 0x00;
			answerData[5U] = 0x40;
#endif
			break;
#ifdef TARGET_ECUC
		case 0x87:
			// intake manifold absolute pressure
			*answerSize = 7U;
			answerData[2U] = 0x01;
			answerData[3U] = 0x0C;
			answerData[4U] = 0x95;
			answerData[5U] = 0x00;
			answerData[6U] = 0x00;
			break;
		case 0x8E:
			// Engine Friction - Percent Torque
			*answerSize = 3U;
			answerData[2U] = 0x80;
			break;
#endif

#ifdef TARGET_ECUD
		case 0x9a:
			// Hybrid/EV Vehicle system data, Battery, Voltage
			*answerSize = 9U;
			answerData[2U] = 0x06;
			answerData[3U] = 0x00;
			answerData[4U] = 0x32;
			answerData[5U] = 0x40;
			answerData[6U] = 0x00;
			answerData[7U] = 0x0E;
			answerData[8U] = 0x00;
			break;
#endif
#ifdef TARGET_ECUC
		case 0x9D:
			// Engine Fuel Rate
			*answerSize = 6U;
			answerData[2U] = 0x00;
			answerData[3U] = 0x00;
			answerData[4U] = 0x00;
			answerData[5U] = 0x00;
			break;
		case 0x9E:
			// Engine Exhaust Flow Rate
			*answerSize = 4U;
			answerData[2U] = 0x00;
			answerData[3U] = 0x00;
			break;
#endif
#ifdef TARGET_ECUB
		case 0xA0:
			// PIDs supported (0xA1 to 0xC0)
			*answerSize = 6U;
			answerData[2U] = 0x04;
			answerData[3U] = 0x00;
			answerData[4U] = 0x00;
			answerData[5U] = 0x00;
			break;
		case 0xA6:
			// Odometer
			*answerSize = 6U;
			answerData[2U] = 0x12;
			answerData[3U] = 0x34;
			answerData[4U] = 0x56;
			answerData[5U] = 0x78;
			break;
#endif
		default:
			formatNegativeResponse(data, answerData, answerSize, J1979_NRC_SFNSIF);
			break;
		}
	}
}

static void showStoredDTCs(const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize)
{
	if (size != 1U) formatNegativeResponse(data, answerData, answerSize, J1979_NRC_SFNSIF);
	else
	{
		// Valid format
#ifdef ENABLE_EEPROM_EMULATION
		uint32_t numDTC;
		if (RAMN_DTC_GetNumberOfDTC(&numDTC) != RAMN_OK)
		{
			formatNegativeResponse(data, answerData, answerSize, J1979_NRC_GR);
		}
		else
		{
			uint32_t tmp;
			*answerSize = 2U + 2U*numDTC;
			answerData[0U] = data[0U] + 0x40;
			answerData[1U] = numDTC&0xFF;

			for(uint16_t i = 0U; i < (numDTC&0xFF); i++ )
			{
				if (RAMN_DTC_GetIndex(i, &tmp) != RAMN_OK)
				{
					formatNegativeResponse(data, answerData, answerSize, J1979_NRC_GR);
					break;
				}
				else
				{
					answerData[2U + 2U*i	 ] = (tmp >> 24)&0xFF;
					answerData[2U + 2U*i + 1U] = (tmp >> 16)&0xFF;
				}
			}
		}
#else
		answerData[0U] = data[0U] + 0x40;
		answerData[1U] = 0x00;
		*answerSize = 2U;
#endif
	}
}

static void clearDTCs(const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize)
{
	if (size != 1U) formatNegativeResponse(data, answerData, answerSize, J1979_NRC_SFNSIF);
	else
	{
#ifdef ENABLE_EEPROM_EMULATION
		if (RAMN_DTC_ClearAll() != RAMN_OK)
		{
			formatNegativeResponse(data, answerData, answerSize, J1979_NRC_GR);
		}
		else
		{
			answerData[0U] = data[0U] + 0x40;
			*answerSize = 1U;
		}
#else
		answerData[0U] = data[0U] + 0x40;
		*answerSize = 1U;
#endif
	}
}

static void requestVehicleInfo(const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize)
{
	if (size != 2U) formatNegativeResponse(data, answerData, answerSize, J1979_NRC_SFNSIF);
	else
	{
		// Valid format
		answerData[0U] = data[0U] + 0x40;
		answerData[1U] = data[1U];
		switch(data[1U])
		{
		case 0x00:
			// Supported PIDs
			answerData[2U] = 0x54;
			answerData[3U] = 0x40;
			answerData[4U] = 0x00;
			answerData[5U] = 0x00;
			*answerSize = 6U;
			break;
		case 0x02:
			// VIN
			answerData[2U] = 0x01;
			*answerSize = 0x14;
#ifdef ENABLE_EEPROM_EMULATION
			EE_Status result = EE_OK;
			result |= RAMN_EEPROM_Read32(VIN_BYTES1_4_INDEX,(uint32_t*)&answerData[3U]);
			result |= RAMN_EEPROM_Read32(VIN_BYTES5_8_INDEX,(uint32_t*)&answerData[7U]);
			result |= RAMN_EEPROM_Read32(VIN_BYTES9_12_INDEX,(uint32_t*)&answerData[11U]);
			result |= RAMN_EEPROM_Read32(VIN_BYTES13_16_INDEX,(uint32_t*)&answerData[15U]);
			result |= RAMN_EEPROM_Read32(VIN_BYTES17_20_INDEX,(uint32_t*)&answerData[19U]); // Only one byte read
			if (result != EE_OK)
			{
				memset(&(answerData[3U]), 0U, 17U);
			}
#else
			memset((uint8_t*)&(answerData[3U]),0U, 17U);
#endif
			break;
		case 0x04:
			// Calibration ID
			answerData[2U] = 0x01;
			strcpy((char*)&answerData[3U],"CALIBRATION ID0"); // 15 chars and zero terminator -> 16 bytes
			*answerSize = 0x13U;
			break;
		case 0x06:
			// CVN
			answerData[2U] = 0x01;
			answerData[3U] = 0x12;
			answerData[4U] = 0x34;
			answerData[5U] = 0x56;
			answerData[6U] = 0x78;
			*answerSize = 7U;
			break;
		case 0x0a:
			// ECU name
			answerData[2U] = 0x01;
			RAMN_memset((char*)&answerData[3U], 0U, 20U); // set all to zero
#if		defined(TARGET_ECUA)
			strcpy((char*)&answerData[3U],"ECUA-Infotainment");
#elif 	defined(TARGET_ECUB)
			strcpy((char*)&answerData[3U],"ECUB-Chassis");
#elif 	defined(TARGET_ECUC)
			strcpy((char*)&answerData[3U],"ECUC-Powertrain");
#elif 	defined(TARGET_ECUD)
			strcpy((char*)&answerData[3U],"ECUD-Body");
#endif
			*answerSize = 0x17U;
			break;
		default:
			formatNegativeResponse(data, answerData, answerSize, J1979_NRC_SFNSIF);
			break;
		}
	}
}

#endif

RAMN_Result_t RAMN_J1979_ProcessMessage(const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize)
{
	switch(data[0])
	{
#ifdef ENABLE_J1979
	case 0x01: // Show Current Data
		showCurrentData(data, size, answerData, answerSize);
		break;
	case 0x03: // Show stored Diagnostic Trouble Codes
	case 0x07: // Show pending Diagnostic Trouble Codes (detected during current or last driving cycle)
	case 0x0A: // Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)
		showStoredDTCs(data, size, answerData, answerSize);
		break;
	case 0x04: // Clear Diagnostic Trouble Codes and stored values
		clearDTCs(data, size, answerData, answerSize);
		break;
	case 0x09: // Request vehicle information
		requestVehicleInfo(data, size, answerData, answerSize);
		break;

	// Services below are not supported
	case 0x02: // Show Freeze Data
	case 0x05: // Test results, oxygen sensor monitoring (non CAN only)
	case 0x06: // Test results, other component/system monitoring (Test results, oxygen sensor monitoring for CAN only)
	case 0x08: // Control operation of on-board component/system
#endif
	default:
		answerData[0] = 0x7F;
		answerData[1] = data[0];
		answerData[2] = J1979_NRC_SNS; // Service Not Supported
		*answerSize = 3;
		break;
	}
	return RAMN_OK;
}

#endif
