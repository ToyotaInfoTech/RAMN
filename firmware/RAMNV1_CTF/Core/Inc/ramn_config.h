/*
 * ramn_config.h
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
 *
 * This software component is licensed by TOYOTA MOTOR CORPORATION under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

//This module holds the configuration of current build

#ifndef INC_RAMN_CONFIG_H_
#define INC_RAMN_CONFIG_H_

#if !defined(TARGET_ECUA) && !defined(TARGET_ECUB) && !defined(TARGET_ECUC) && !defined(TARGET_ECUD)
//#define TARGET_ECUA
#define TARGET_ECUB
//#define TARGET_ECUC
//#define TARGET_ECUD
#endif

#ifdef TARGET_ECUA
#error ECU A NOT COMPATIBLE WITH CTF FIRMWARE (use standard firmware without memory protection)
#endif

//Loop time for the "simulator" that is executed periodically
#define SIM_LOOP_CLOCK_MS 10

//Enable watchdogs - Application needs to kick it every 1s
//#define WATCHDOG_ENABLE


#if defined(TARGET_ECUB)
//#define ENABLE_SCREEN
#define EXPANSION_CHASSIS
//#define ENABLE_REPROGRAMMING
#define ENABLE_UDS
//#define ENABLE_KWP
//#define ENABLE_XCP
#endif

#if defined(TARGET_ECUC)
//#define ENABLE_SCREEN
#define EXPANSION_POWERTRAIN
//#define ENABLE_REPROGRAMMING
#define ENABLE_UDS
//#define ENABLE_KWP
//#define ENABLE_XCP
#endif

#if defined(TARGET_ECUD)
#define EXPANSION_BODY
//#define ENABLE_REPROGRAMMING
#define ENABLE_UDS
//#define ENABLE_KWP
//#define ENABLE_XCP
#endif

#if defined(ENABLE_UDS) || defined(ENABLE_KWP) || defined(ENABLE_XCP)
#define ENABLE_DIAG
//#define ENABLE_EEPROM_EMULATION
#endif

#if defined(ENABLE_UDS) || defined(ENABLE_KWP)
#define ENABLE_ISOTP
#endif

#define ISOTP_RXBUFFER_SIZE 			4096
#define ISOTP_TXBUFFER_SIZE 			4096
#define ISOTP_CONSECUTIVE_BLOCK_SIZE 	0
#define ISOTP_CONSECUTIVE_ST 			0
#define ISOTP_RX_TIMEOUT_MS 			200000
#define ISOTP_TX_TIMEOUT_MS 			200000
#define TRNG_POOL_SIZE 	   				256

#define CAN_RX_BUFFER_SIZE 				4096
#define CAN_TX_BUFFER_SIZE 				4096
#define UDS_ISOTP_RX_BUFFER_SIZE 		(4200)
#define UDS_ISOTP_TX_BUFFER_SIZE 		(4200)
#define KWP_ISOTP_RX_BUFFER_SIZE 		(0)
#define KWP_ISOTP_TX_BUFFER_SIZE 		(0)
#define XCP_RX_BUFFER_SIZE 				(0)
#define XCP_TX_BUFFER_SIZE 				(0)

#endif /* INC_RAMN_CONFIG_H_ */
