/*
 * ramn_config.h
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

//This module holds the configuration of current build

#ifndef INC_RAMN_CONFIG_H_
#define INC_RAMN_CONFIG_H_

#if !defined(TARGET_ECUA) && !defined(TARGET_ECUB) && !defined(TARGET_ECUC) && !defined(TARGET_ECUD)
#define TARGET_ECUA
//#define TARGET_ECUB
//#define TARGET_ECUC
//#define TARGET_ECUD
#endif



//Loop time for the "simulator" that is executed periodically
#define SIM_LOOP_CLOCK_MS 10

//Use big endian for CAN brake/steering/accelerator sensors instead of ARM Little Endian
#define USE_BIG_ENDIAN_CAN

//Enable watchdogs - Application needs to kick it every 1s
//#define WATCHDOG_ENABLE

#define UDS_ACCEPT_FUNCTIONAL_ADDRESSING //Value must be defined in ramn_vehicle_specific.h

#if defined(TARGET_ECUA)
#define ENABLE_USB
#define ENABLE_SCREEN
#define ENABLE_MINICTF


//Automatically stops transceiving when serial port close is detected. May not work well with some OS/Applications
//#define ENABLE_USB_AUTODETECT

//#define START_IN_CLI_MODE  // will start in CLI mode instead of slcan if enabled

//This define can be used to reset peripheral if it enters bus-off mode (not recommended unless you know what you are doing)
//#define AUTO_RECOVER_BUSOFF

//#define GENERATE_RUNTIME_STATS //requires to add "volatile" keyword to static uint32_t ulTotalRunTime = 0UL;
//If CAN_ECHO is enabled, ECU A will repeat whatever message it accepts over USB
// CAN_ECHO does not cover CAN messages not sent from USB (i.e., answer to UDS commands)
//#define CAN_ECHO

//Define this to let ECU A process slcan message that it receives as regular messages (and update their value on screen, for example)
//This is useful to demonstrate the impact of CAN fuzzing on ECU A's screen
#define PROCESS_SLCAN_BY_DBC

//UDS programming does not work on microcontrollers with only 256 kb memory
#define ENABLE_REPROGRAMMING
#define ENABLE_UDS
//#define ENABLE_KWP
//#define ENABLE_XCP
#define RTR_DEMO_ID 0x700
#endif

#if defined(TARGET_ECUB)
//#define ENABLE_SCREEN
#define EXPANSION_CHASSIS


//#define CHASSIS_LINEAR_POTENTIOMETER
#if !defined(CHASSIS_LOGARITHMIC_POTENTIOMETER) && !defined(CHASSIS_LINEAR_POTENTIOMETER)
#define CHASSIS_LOGARITHMIC_POTENTIOMETER
#endif

#define ENABLE_REPROGRAMMING
#define ENABLE_UDS
//#define ENABLE_KWP
#define ENABLE_XCP
#define RTR_DEMO_ID 0x701
#endif

#if defined(TARGET_ECUC)
//#define ENABLE_SCREEN
#define EXPANSION_POWERTRAIN
#define ENABLE_REPROGRAMMING
#define ENABLE_UDS
//#define ENABLE_KWP
#define ENABLE_XCP
#define RTR_DEMO_ID 0x702
#endif

#if defined(TARGET_ECUD)
#define ENABLE_MINICTF
#define EXPANSION_BODY
#define ENABLE_REPROGRAMMING
#define ENABLE_UDS
//#define ENABLE_KWP
#define ENABLE_XCP
#define RTR_DEMO_ID 0x703
//enable define below to force LEDs to light up for 3 seconds at startup
#define PERFORM_STARTUP_LED_TEST
#endif

#if defined(ENABLE_UDS) || defined(ENABLE_KWP) || defined(ENABLE_XCP)
#define ENABLE_DIAG
#define ENABLE_EEPROM_EMULATION
#if defined(ENABLE_SCREEN)
#define UDS_DRAW_BUFFER_SIZE 0x1000
#endif
#endif

#if defined(ENABLE_UDS) || defined(ENABLE_KWP)
#define ENABLE_ISOTP
#endif

#define ISOTP_RXBUFFER_SIZE 			4096
#define ISOTP_TXBUFFER_SIZE 			4096
#define ISOTP_CONSECUTIVE_BLOCK_SIZE 	0
#define ISOTP_CONSECUTIVE_ST 			0
#define ISOTP_RX_TIMEOUT_MS 			200000
#define ISOTP_TX_TIMEOUT_MS 			2000

#define TRNG_POOL_SIZE 	   				256
#define JOYSTICK_POOL_SIZE				10


#ifdef TARGET_ECUA
#define USB_RX_BUFFER_SIZE 				18000
#define USB_TX_BUFFER_SIZE 				18000
#define CAN_RX_BUFFER_SIZE 				23000
#define CAN_TX_BUFFER_SIZE 				18000
#define UDS_ISOTP_RX_BUFFER_SIZE 		0xFFF+2 //Add +2 for buffer-size
#define UDS_ISOTP_TX_BUFFER_SIZE 		0xFFF+2

#else
#define CAN_RX_BUFFER_SIZE 				20480
#define CAN_TX_BUFFER_SIZE 				20480
#define UDS_ISOTP_RX_BUFFER_SIZE 		(4097*6)
#define UDS_ISOTP_TX_BUFFER_SIZE 		(4097*6)
#define KWP_ISOTP_RX_BUFFER_SIZE 		(4097*6)
#define KWP_ISOTP_TX_BUFFER_SIZE 		(4097*6)
#define XCP_RX_BUFFER_SIZE 				(4000)
#define XCP_TX_BUFFER_SIZE 				(4000)
#endif


#endif /* INC_RAMN_CONFIG_H_ */
