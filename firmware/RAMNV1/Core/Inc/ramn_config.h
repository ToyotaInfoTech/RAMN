/*
 * ramn_config.h
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

// This module holds the configuration of current build.

#ifndef INC_RAMN_CONFIG_H_
#define INC_RAMN_CONFIG_H_

// Define which ECU the source code should be built for.
#if !defined(TARGET_ECUA) && !defined(TARGET_ECUB) && !defined(TARGET_ECUC) && !defined(TARGET_ECUD)
#define TARGET_ECUA
//#define TARGET_ECUB
//#define TARGET_ECUC
//#define TARGET_ECUD
#endif

// See comments at the end of file if you want to use the internal oscillator instead of the external crystal.

// CONFIGURATION OF ECU A ------------------------------------------------------

#if defined(TARGET_ECUA)
#define ENABLE_USB
#define ENABLE_CDC // USB serial (CDC) interface

// Enable this to enable the candlelight interface (gs_usb drivers)
// Current implementation is experimental:
// - Error frames are not reported
// - CAN-FD is not supported
// - Due to clock differences, bit timings are not respected (but equivalent baudrates are used)
// You may want to update USBD_VID and USBD_PID in usbd_desc.c to automatically load the drivers on Linux.
//#define ENABLE_GSUSB

// Enable screen (not restricted to ECU A).
#define ENABLE_SCREEN

// Enable Chip 8 engine.
#define ENABLE_CHIP8

// Enable USB MiniCTF challenges.
#define ENABLE_MINICTF

// Note that UDS programming does not work on microcontrollers with only 256 kb memory (e.g., reference ending with CCT6).
#define ENABLE_REPROGRAMMING
#define ENABLE_UDS
//#define ENABLE_KWP
//#define ENABLE_XCP
#define RTR_DEMO_ID 0x700

// ECU will monitor joystick CAN messages and generate events such as PRESSED and RELEASED.
// You must independently make sure that the ECU does not filter out the relevant CAN messages (which it shouldn't by default).
#define ENABLE_JOYSTICK_CONTROLS

// Use this to define a "password" for the command to put ECU A back into DFU mode.
// Note that this is not for security (because STM32 can be put back into DFU mode over JTAG, and if JTAG is permanently disabled (level 2) the back-to-DFU command won't work correctly anyway).
// This is only to prevent users from accidentally putting the ECU back into DFU mode (e.g. because of fuzzing).
#define DFU_COMMAND_STRING "zZ"

// Automatically close the serial port if it overflows (consider that the user forgot to close it).
#define CLOSE_DEVICE_ON_USB_TX_OVERFLOW

// Automatically stops transmitting when serial port close is detected.
// May not work well with some OS/Applications
// #define ENABLE_USB_AUTODETECT

// Will start the usb serial interface in CLI mode instead of slcan if enabled.
//#define START_IN_CLI_MODE

// Enable this flag to enable FreeRTOS runtime stats.
// This also requires to add "volatile" keyword to static uint32_t ulTotalRunTime = 0UL in tasks.c of FreeRTOS (typically overwritten by STM32CubeIDE code generation).
// #define GENERATE_RUNTIME_STATS

// If this flag is enabled, ECU A will repeat whatever message it accepts over USB.
// May be useful when multiplexing the serial interface, but should typically not be used.
// CAN_ECHO does not cover ECU A CAN messages not sent from USB (i.e., answer to UDS commands).
//#define CAN_ECHO

// Define this flag to let ECU A process slcan message that it receives as regular RX messages (and update their value on screen, for example).
// This is useful to demonstrate the impact of CAN fuzzing on ECU A's screen when using ECU A's slcan interface, even though ECU A did not actually receive the fuzzed CAN message (since it was the transmitter).
#define PROCESS_SLCAN_BY_DBC

// Define this flag to enable the USB debugging module.
// Note that it also needs to be activated by a slcan command, or by setting RAMN_DEBUG_ENABLE in ramn_debug.c to True.
#define ENABLE_USB_DEBUG

// Number of times to retry entering bootloader mode of another ECU before giving up
#define BOOTLOADER_MAX_ATTEMPTS 20


#ifdef ENABLE_GSUSB
#define GSUSB_RECV_QUEUE_SIZE 512 // Size of the GSUSB receive queue (uint32_t elements)
#define GSUSB_POOL_QUEUE_SIZE 512 // Size of the GSUSB pool queue (uint32_t elements)
#define GSUSB_SEND_QUEUE_SIZE 512 // Size of the GSUSB send queue (uint32_t elements)
#define GS_HOST_FRAME_SIZE     80
#endif

#endif

// CONFIGURATION OF ECU B ------------------------------------------------------

#if defined(TARGET_ECUB)
//#define ENABLE_SCREEN
#define EXPANSION_CHASSIS

// Define flag below to indicate that the steering wheel potentiometer is of the logarithmic type.
//#define CHASSIS_LOGARITHMIC_POTENTIOMETER
#if !defined(CHASSIS_LOGARITHMIC_POTENTIOMETER) && !defined(CHASSIS_LINEAR_POTENTIOMETER)
// If CHASSIS_LINEAR_POTENTIOMETER not defined by build script, consider Log as default
#define CHASSIS_LOGARITHMIC_POTENTIOMETER
#endif

#define ENABLE_REPROGRAMMING
#define ENABLE_UDS
//#define ENABLE_KWP
#define ENABLE_XCP
#define RTR_DEMO_ID 0x701
#endif

// CONFIGURATION OF ECU C ------------------------------------------------------

#if defined(TARGET_ECUC)
//#define ENABLE_SCREEN
#define EXPANSION_POWERTRAIN
#define ENABLE_REPROGRAMMING
#define ENABLE_UDS
//#define ENABLE_KWP
#define ENABLE_XCP
#define RTR_DEMO_ID 0x702
#endif

// CONFIGURATION OF ECU D ------------------------------------------------------

#if defined(TARGET_ECUD)
#define ENABLE_MINICTF
#define EXPANSION_BODY
#define ENABLE_REPROGRAMMING
#define ENABLE_UDS
//#define ENABLE_KWP
#define ENABLE_XCP
#define RTR_DEMO_ID 0x703

// How long to light up ECU D's LEDs at startup. Set to 0 to skip test.
#define LED_TEST_DURATION_MS 3000U
#endif

// Common configuration ------------------------------------------------------

// Loop time for the "simulator" that is executed periodically.
#define SIM_LOOP_CLOCK_MS 10

// Enable watchdogs - Application needs to kick it every 1s.
// #define WATCHDOG_ENABLE

// Will make ECU accept broadcasted UDS commands.
// CAN ID must be defined in ramn_vehicle_specific.h.
#define UDS_ACCEPT_FUNCTIONAL_ADDRESSING

// Enable dynamic computations of tseg1 and tseg2 to get non-standard baudrate.
// /!\CURRENTLY UNTESTED
#define ENABLE_DYNAMIC_BITRATE

// This flag can be used to automatically reset CAN/CAN-FD peripheral if it enters bus-off mode.
// #define AUTO_RECOVER_BUSOFF

// Use big endian for CAN brake/steering/accelerator sensors instead of ARM Little Endian.
#define USE_BIG_ENDIAN_CAN

#ifndef TARGET_ECUA
// Use Hardware CAN filters (Up to 28 standard IDs, 8 extended IDs).
// If this is enabled, you need to update ECU RX filters when you want to add CAN IDs to the network.
#define USE_HARDWARE_CAN_FILTERS
#endif

// Enable these flags to display a warning if the periodic task is taking longer than the defined period.
// #define DISPLAY_SLOW_WARNING

// If the flag above is defined, you can decide to skip the first cycles, which are expected to take longer due to initializations.
#define SLOW_WARNING_MIN_LOOP_COUNT 1U

// Enable the I2C interface
// /!\CURRENTLY UNTESTED
// #define ENABLE_I2C

// Enable the UART interface
// /!\ CURRENTLY UNTESTED
// #define ENABLE_UART

// Enable this flag to force code to while(1) when encountering errors that could typically be ignored.
// #define HANG_ON_ERRORS

// Enable this flag to automatically enable memory protection at startup. Once activated, it can only be removed by bootloader/usb commands.
// Avoid using if you are not sure what you are doing.
// #define MEMORY_AUTOLOCK

// Value to set to the RDP option byte if flag above is active.
// It is 0xAA when unlocked (OB_RDP_LEVEL_0), 0xBB if temporarily locked by bootloader (OB_RDP_LEVEL_1, or 0xDC if locked by STM32CubeProgrammer).
// Setting this value to 0xCC (OB_RDP_LEVEL_2) will PERMANENTLY LOCK JTAG (the ECU will not be reprogrammable anymore)
#define RDP_OPTIONBYTE OB_RDP_LEVEL_1

// If this is defined, the TRNG module will fill a stream buffer with random bytes instead of calling the HAL library.
//#define USE_TRNG_BUFFER

#ifdef ENABLE_I2C
#define I2C_RX_BUFFER_SIZE				(16)
#define I2C_TX_BUFFER_SIZE				(4)
#endif

#ifdef ENABLE_UART

// UART module works by queuing commands in a stream buffer.
// You should define the size of working buffers (for currently processed command/answers) and of stream buffers (where all commands/queued are queued and waiting for transmission).

// Working buffer
#define UART_RX_COMMAND_BUFFER_SIZE			128 //This is the maximum length of a command to receive, used by both TASK and ISR
#define UART_TX_COMMAND_BUFFER_SIZE			128 //This is the maximum length of a command to send back

// Stream buffer
#define UART_RX_BUFFER_SIZE 				512
#define UART_TX_BUFFER_SIZE 				512

#if (UART_RX_COMMAND_BUFFER_SIZE > UART_RX_BUFFER_SIZE) || (UART_TX_COMMAND_BUFFER_SIZE > UART_TX_BUFFER_SIZE)
#error command size should be smaller than stream buffer size
#endif

#endif


#if defined(ENABLE_UDS) || defined(ENABLE_KWP) || defined(ENABLE_XCP)
#define ENABLE_DIAG
#define ENABLE_EEPROM_EMULATION
#if defined(ENABLE_SCREEN)
// Size of the buffer used to draw on screen using UDS commands
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
#define USB_RX_BUFFER_SIZE 				15000
#define USB_TX_BUFFER_SIZE 				15000
#define CAN_RX_BUFFER_SIZE 				15000
#define CAN_TX_BUFFER_SIZE 				15000
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

// This Extended ID is set as a hardware filter example event when CTF is disabled.
#define CTF_EXTENDED_ID 0x77cafe
#ifdef ENABLE_MINICTF
#define CTF_STANDARD_ID_1 0x456
#define CTF_STANDARD_ID_2 0x772
#define CTF_STANDARD_ID_3 0x457
#define CTF_STANDARD_ID_4 0x458
#endif

// Check for bad configurations --------------------------------------

#if defined(TARGET_ECUA) + defined(TARGET_ECUB) + defined(TARGET_ECUC) + defined(TARGET_ECUD) != 1
    #error "You must define only one of TARGET_ECUA, TARGET_ECUB, TARGET_ECUC, and TARGET_ECUD."
#endif

#if defined(ENABLE_UART) && defined(ENABLE_USB)
#error Default code does not support UART and USB at the same time. See comments for details.
// UART uses USB task by default, and therefore USB and UART cannot be used at the same time.
// You can enable both simultaneously by creating new receive/transmit tasks for UART, and move the UART code (between #define ENABLE_UART) in RAMN_ReceiveUSBFunc and RAMN_SendUSBFunc there.
// You should then modify HAL_UART_TxCpltCallback and HAL_UART_RxCpltCallback to notify these tasks instead.
#endif

#if defined(ENABLE_CHIP8) && !defined(ENABLE_SCREEN)
#error ENABLE_SCREEN must be defined to define ENABLE_CHIP8
#endif

#if !defined(ENABLE_USB) && defined(ENABLE_CDC)
#error Cannot activate CDC without enabling USB
#endif

#if !defined(ENABLE_USB) && defined(ENABLE_GSUSB)
#error Cannot activate GSUSB without enabling USB
#endif

#if defined(ENABLE_USB) && !defined(ENABLE_CDC) && !defined(ENABLE_GSUSB)
#error At least one USB interface must be active if you enable USB
#endif

// To use the internal oscillator (instead of the default external 10MHz crystal), You should modify RAMNV1.ioc so that:
// - PLL Source Mux uses HSI with *N = X 10
// - PLLSAI1 Source Mux uses HSI with *N = X 15
// You should then regenerate the source code (refer to docs/firmware/img/internal_clock_settings.png and external_clock_settings.png to compare clock settings).
// If you do not want to use the code generation feature,you can also modify the source code directly so that the following values are used instead of default values:
//
// In stm32l5xx_hal_msp.c:
// PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSAI1SOURCE_HSI;
// PeriphClkInit.PLLSAI1.PLLSAI1N = 15;
//
// In main.c:
//
// RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
// RCC_OscInitStruct.HSIState = RCC_HSI_ON;
// RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
// RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
// RCC_OscInitStruct.PLL.PLLN = 10;

#endif /* INC_RAMN_CONFIG_H_ */
