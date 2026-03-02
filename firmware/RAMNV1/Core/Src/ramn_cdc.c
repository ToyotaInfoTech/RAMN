/*
 * ramn_cdc.c
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

#include "ramn_cdc.h"

#ifdef ENABLE_CDC

#ifdef ENABLE_UDS
// Holds currently processed Diag Command from USB.
static __attribute__ ((section (".buffers")))  uint8_t diagRxUSBbuf[0xFFF+2];
// Holds currently processed Diag Command Answer from USB.
static __attribute__ ((section (".buffers")))  uint8_t diagTxUSBbuf[0xFFF+2];
#endif

#ifdef GENERATE_RUNTIME_STATS
static __attribute__ ((section (".buffers"))) TaskStatus_t pxTaskStatusArray[MAX_NUMBER_OF_TASKS];
#endif

static int countElements(char* buffer, int length) {
	int count = 0;
	int inWord = 0;

	for (int i = 0; i < length; i++) {
		if (buffer[i] == ' ') {
			inWord = 0;  // Not in a word
		} else if (inWord == 0) {
			inWord = 1;  // Start of a new word
			count++;
		}
	}
	return count;
}

RAMN_Bool_t RAMN_CDC_ProcessCLIBuffer(uint8_t* USBRxBuffer, uint32_t commandLength)
{
	// Command is in USBRxBuffer, length is in commandLength. There is no endline in buffer.
	RAMN_Bool_t mustSwitch = False; //return value, set to True if device should switch to slcan mode

	// Zero terminate the USB command buffer
	USBRxBuffer[commandLength] = '\0';

	if (commandLength >= USB_COMMAND_BUFFER_SIZE) RAMN_USB_SendStringFromTask("Command too long.\r");
	else
	{
		// Must remove backspace
		uint32_t processedLength = 0;
		uint32_t elementCount = 0;

		for (uint32_t k = 0; k < commandLength; k++)
		{
			if (USBRxBuffer[k] == '\b')
			{
				if (processedLength > 0)
				{
					processedLength--; // Remove previous character
				}
			}
			else if (USBRxBuffer[k] != '\n')
			{
				USBRxBuffer[processedLength] = USBRxBuffer[k];
				processedLength++;
			}
		}

		// Null-terminate the processed command
		USBRxBuffer[processedLength] = '\0';
		commandLength = processedLength;

		elementCount = countElements((char*)USBRxBuffer, commandLength);

		if (elementCount == 0U && commandLength == 0U) RAMN_USB_SendStringFromTask("\r>\r");
		else
		{
			char *token;

			token = strtok((char*)USBRxBuffer, " ");

			if (token == NULL) {
				RAMN_USB_SendStringFromTask("No command found. Type \"help\" for help.\r");
			}
			else
			{
				// Compare the command to a set of possible commands
				if (strcmp(token, "help") == 0 || strcmp(token, "Help") == 0 || strcmp(token, "HELP") == 0 || strcmp(token, "man") == 0) {

					if (elementCount >= 2)
					{
						token = strtok(NULL, " ");
						if (strcmp(token, "theme") == 0) {
							RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
							RAMN_USB_SendStringFromTask("                      RAMN Command Help\r");
							RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Command: theme\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Description:\r");
							RAMN_USB_SendStringFromTask("    Set the color theme for the device LCD screen.\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Usage:\r");
							RAMN_USB_SendStringFromTask("    theme <theme_number>\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Options:\r");
							RAMN_USB_SendStringFromTask("    <theme_number>   The theme number to be set, ranging from 1 to 7.\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Examples:\r");
							RAMN_USB_SendStringFromTask("    - To set the device LCD screen theme to theme number 3, enter:\r");
							RAMN_USB_SendStringFromTask("      theme 3\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
						}
						else if (strcmp(token, "enable") == 0) {
							RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
							RAMN_USB_SendStringFromTask("                      RAMN Command Help\r");
							RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Command: enable\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Description:\r");
							RAMN_USB_SendStringFromTask("    Enable the power supply of another ECU (B, C, or D).\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Usage:\r");
							RAMN_USB_SendStringFromTask("    enable <ECU>\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Options:\r");
							RAMN_USB_SendStringFromTask("    <ECU>   The ECU to be enabled, identified by a letter (B, C, or D).\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Examples:\r");
							RAMN_USB_SendStringFromTask("    - To enable ECU B, use:\r");
							RAMN_USB_SendStringFromTask("      enable B\r");
							RAMN_USB_SendStringFromTask("    - To enable ECU C, use:\r");
							RAMN_USB_SendStringFromTask("      enable C\r");
							RAMN_USB_SendStringFromTask("    - To enable ECU D, use:\r");
							RAMN_USB_SendStringFromTask("      enable D\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
						}
						else if (strcmp(token, "disable") == 0) {
							RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
							RAMN_USB_SendStringFromTask("                      RAMN Command Help\r");
							RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Command: disable\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Description:\r");
							RAMN_USB_SendStringFromTask("    Disable the power supply of another ECU (B, C, or D).\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Usage:\r");
							RAMN_USB_SendStringFromTask("    disable <ECU>\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Options:\r");
							RAMN_USB_SendStringFromTask("    <ECU>   The ECU to be disabled, identified by a letter (B, C, or D).\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("Examples:\r");
							RAMN_USB_SendStringFromTask("    - To disable ECU B, use:\r");
							RAMN_USB_SendStringFromTask("      disable B\r");
							RAMN_USB_SendStringFromTask("    - To disable ECU C, use:\r");
							RAMN_USB_SendStringFromTask("      disable C\r");
							RAMN_USB_SendStringFromTask("    - To disable ECU D, use:\r");
							RAMN_USB_SendStringFromTask("      disable D\r");
							RAMN_USB_SendStringFromTask("\r");
							RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
						}
						else
						{
							RAMN_USB_SendStringFromTask("No help page found for this command.\r");
						}
						RAMN_USB_SendStringFromTask("End of command help page. Type \"help\" without argument for the general help page.\r");
					}
					else
					{
						RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
						RAMN_USB_SendStringFromTask("                      RAMN Command Help\r");
						RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
						RAMN_USB_SendStringFromTask("\r");
						RAMN_USB_SendStringFromTask("This interface allows you to interact with various RAMN functions and control the device's operations through a set of commands.\rType 'b' to go back to slcan mode.");
						RAMN_USB_SendStringFromTask("\r");
						RAMN_USB_SendStringFromTask("Commands:\r");
						RAMN_USB_SendStringFromTask("    - clear: Clears your serial terminal.\r");
						RAMN_USB_SendStringFromTask("    - disable: Disable the power supply for another ECU. Usage: disable <ECU>.\r");
						RAMN_USB_SendStringFromTask("    - enable: Enable the power supply for another ECU. Usage: enable <ECU>.\r");
						RAMN_USB_SendStringFromTask("    - exit: Exit this debug interface and revert to slcan mode. Usage: exit.\r");
						RAMN_USB_SendStringFromTask("    - help: Display general help, or help for a specific command when available. Usage: help <command>.\r");
						RAMN_USB_SendStringFromTask("    - b: Alias for the \"exit\" command.\r");
						RAMN_USB_SendStringFromTask("    - quit: Alias for the \"exit\" command.\r");
						RAMN_USB_SendStringFromTask("    - reset: Reset the device. Usage: reset.\r");
						RAMN_USB_SendStringFromTask("    - slcan: Alias for the \"exit\" command.\r");
						RAMN_USB_SendStringFromTask("    - theme: Set the color theme for ECU A's LCD screen. Usage: theme <theme number>.\r");
#ifdef ENABLE_CHIP8
						RAMN_USB_SendStringFromTask("    - play: Play a game on ECU A's LCD screen. Usage: play <game number>.\r");
						RAMN_USB_SendStringFromTask("    - stop: Stop any ongoing game. Usage: stop.\r");
#endif
						RAMN_USB_SendStringFromTask("\r");
						RAMN_USB_SendStringFromTask("Commands are case sensitive.\r");
						RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
					}
				}
				else if ( strcmp(token, "disable") == 0) {
					if (elementCount != 2)
					{
						RAMN_USB_SendStringFromTask("Invalid number of arguments. Type \"help disable\" for help using this command.\r");
					}
					else
					{
						token = strtok(NULL, " ");
						if (strcmp(token, "A") == 0) {
							RAMN_USB_SendStringFromTask("Cannot disable ECU A. Type \"reset\" if reset it.\r");
						} else if (strcmp(token, "B") == 0)
						{
							RAMN_USB_SendStringFromTask("Setting ECU B power supply to OFF.\r");
							RAMN_ECU_SetEnable('B',GPIO_PIN_RESET);
						} else if (strcmp(token, "C") == 0)
						{
							RAMN_USB_SendStringFromTask("Setting ECU C power supply to OFF.\r");
							RAMN_ECU_SetEnable('C',GPIO_PIN_RESET);
						}
						else if (strcmp(token, "D") == 0)
						{
							RAMN_USB_SendStringFromTask("Setting ECU D power supply to OFF.\r");
							RAMN_ECU_SetEnable('D',GPIO_PIN_RESET);
						}
						else
						{
							RAMN_USB_SendStringFromTask("Invalid ECU. Must be B, C, or D.\r");
						}
					}
				}
				else if ( strcmp(token, "enable") == 0) {
					if (elementCount != 2)
					{
						RAMN_USB_SendStringFromTask("Invalid number of arguments. Type \"help enable\" for help using this command.\r");
					}
					else
					{
						token = strtok(NULL, " ");
						if (strcmp(token, "A") == 0) {
							RAMN_USB_SendStringFromTask("ECU A is always enabled.\r");
						}
						else if (strcmp(token, "B") == 0)
						{
							RAMN_USB_SendStringFromTask("Setting ECU B power supply to ON.\r");
							RAMN_ECU_SetEnable('B',GPIO_PIN_SET);
						} else if (strcmp(token, "C") == 0)
						{
							RAMN_USB_SendStringFromTask("Setting ECU C power supply to ON.\r");
							RAMN_ECU_SetEnable('C',GPIO_PIN_SET);
						}
						else if (strcmp(token, "D") == 0)
						{
							RAMN_USB_SendStringFromTask("Setting ECU D power supply to ON.\r");
							RAMN_ECU_SetEnable('D',GPIO_PIN_SET);
						}
						else
						{
							RAMN_USB_SendStringFromTask("Invalid ECU. Must be B, C, or D.\r");
						}
					}

				}
#ifdef ENABLE_SCREEN
				else if ( strcmp(token, "theme") == 0) {
					if (elementCount != 2)
					{
						RAMN_USB_SendStringFromTask("Invalid number of arguments. Type \"help theme\" for help using this command.\r");
					}
					else
					{
						token = strtok(NULL, " ");
						if (strcmp(token, "1") == 0) {  RAMN_SCREENUTILS_UpdateTheme(1); RAMN_USB_SendStringFromTask("Updated.\r");
						} else if (strcmp(token, "2") == 0) {  RAMN_SCREENUTILS_UpdateTheme(2); RAMN_USB_SendStringFromTask("Updated.\r");
						} else if (strcmp(token, "3") == 0) {  RAMN_SCREENUTILS_UpdateTheme(3); RAMN_USB_SendStringFromTask("Updated.\r");
						} else if (strcmp(token, "4") == 0) {  RAMN_SCREENUTILS_UpdateTheme(4); RAMN_USB_SendStringFromTask("Updated.\r");
						} else if (strcmp(token, "5") == 0) {  RAMN_SCREENUTILS_UpdateTheme(5); RAMN_USB_SendStringFromTask("Updated.\r");
						} else if (strcmp(token, "6") == 0) {  RAMN_SCREENUTILS_UpdateTheme(6); RAMN_USB_SendStringFromTask("Updated.\r");
						} else if (strcmp(token, "7") == 0) {  RAMN_SCREENUTILS_UpdateTheme(7); RAMN_USB_SendStringFromTask("Updated.\r");

						}
						else
						{
							RAMN_USB_SendStringFromTask("Theme number not found. Try between 1 and 7.\r");
						}
					}
				}
#endif
				else if (strcmp(token, "b") == 0  || strcmp(token, "exit") == 0 || strcmp(token, "quit") == 0 || strcmp(token, "slcan") == 0) {
					mustSwitch = True;
				}

				else if (strcmp(token, "reset") == 0) {

#ifdef START_IN_CLI_MODE
					RAMN_USB_SendStringFromTask("Resetting...\r\r>");
#else
					RAMN_USB_SendStringFromTask("Resetting. Remember to first send the \"#\" command to reenter this interface.\r");
#endif
					osDelay(200);
					HAL_NVIC_SystemReset();
				}
				else if (strcmp(token, "clear") == 0) {
					RAMN_USB_SendStringFromTask("\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r");
				}
#if defined(ENABLE_SCREEN) && defined(ENABLE_CHIP8)
				else if (strcmp(token, "play") == 0)
				{
					if (elementCount != 2)
					{
						RAMN_USB_SendStringFromTask("Invalid number of arguments.\r");
					}
					else
					{
						token = strtok(NULL, " ");
						if (strcmp(token, "1") == 0) {  RAMN_SCREENMANAGER_StartGameFromIndex(1); RAMN_USB_SendStringFromTask("Starting game 1.\r");
						} else if (strcmp(token, "2") == 0) {  RAMN_SCREENMANAGER_StartGameFromIndex(2); RAMN_USB_SendStringFromTask("Starting game 2.\r");
						} else if (strcmp(token, "3") == 0) {  RAMN_SCREENMANAGER_StartGameFromIndex(3); RAMN_USB_SendStringFromTask("Starting game 3.\r");
						}
						else
						{
							RAMN_USB_SendStringFromTask("Game number not found. Try between 1 and 3.\r");
						}
					}

				}
				else if (strcmp(token, "stop") == 0U)
				{
					if (RAMN_CHIP8_IsGameActive() != False)
					{
						RAMN_USB_SendStringFromTask("Stopping game.\r");
						RAMN_CHIP8_StopGame(1);
					}
					else
					{
						RAMN_USB_SendStringFromTask("No ongoing game.\r");
					}
				}
#endif
				else {
					// Handle unknown commands
					RAMN_USB_SendStringFromTask("Unknown command: ");
					RAMN_USB_SendStringFromTask(token);
					RAMN_USB_SendStringFromTask("\rType \"help\" for help. Remember the interface is case sensitive.\r");
				}

			}
		}

	}
	if (!mustSwitch)  RAMN_USB_SendStringFromTask("\r>");
	return mustSwitch;
}



RAMN_Bool_t RAMN_CDC_ProcessSLCANBuffer(uint8_t* USBRxBuffer, uint32_t commandLength)
{

	FDCAN_TxHeaderTypeDef CANTxHeader;
	uint8_t CANTxData[64];
	FDCAN_ProtocolStatusTypeDef protocolStatus;
	FDCAN_ErrorCountersTypeDef errorCount;

	uint8_t dlc;
	uint8_t offset = 0U; //TODO: remove use of offset by using pointers and dedicated functions
	uint8_t smallResponseBuffer[50U]; // Buffer for small responses

	RAMN_Bool_t mustSwitch = False; //return value, set to True if device should switch to CLI mode

	if ((USBRxBuffer[0U] == '0') || (USBRxBuffer[0U] == '1'))
	{
		// Prefix to announce FD CAN Frames
		CANTxHeader.FDFormat = FDCAN_FD_CAN;
		if (USBRxBuffer[1U] == '0') CANTxHeader.BitRateSwitch = FDCAN_BRS_OFF;
		else CANTxHeader.BitRateSwitch = FDCAN_BRS_ON;
		offset = 1U;
	}
	else
	{
		CANTxHeader.FDFormat = FDCAN_CLASSIC_CAN;
		CANTxHeader.BitRateSwitch = FDCAN_BRS_OFF;
		offset = 0U;
	}
	// Parse the 'i' suffix if it was added to alter the Error State Indicator
	if (RAMN_USB_Config.addESIFlag == True)
	{
		if(USBRxBuffer[commandLength-1] == 'i')
		{
			CANTxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
			commandLength--;
		}
		else CANTxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	}
	else CANTxHeader.ErrorStateIndicator = RAMN_FDCAN_Status.ErrorStateIndicator;

	CANTxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	CANTxHeader.MessageMarker = 0U;

	// Sending and Receiving are the most likely commands, so check for them first

	// Most common commands are treated first
	if(USBRxBuffer[0U] == 'u')
	{
		// Format is u<brake12><accel12><rpm12><steer12><shift8><horn8><handbrake8>
		// e.g., "u{:03x}{:03x}{:03x}{:03x}{:02x}{:02x}{:02x}\r"
		if (commandLength == 19U) RAMN_DBC_ProcessUSBBuffer(USBRxBuffer);
	}
	else if ( ((USBRxBuffer[0U+offset] == 't') || (USBRxBuffer[0U+offset] == 'r')))
	{
		// 't' : Transmit Standard ID DATA
		// 'r' : Transmit Standard ID RTR
		if (((commandLength - offset) >= 5U))
		{
			CANTxHeader.IdType = FDCAN_STANDARD_ID;
			CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
			//CANTxHeader.Identifier = (ascii_hashmap[commandBuffer[1+offset]] << 8) + (ascii_hashmap[commandBuffer[2+offset]] << 4) + (ascii_hashmap[commandBuffer[3+offset]]);
			CANTxHeader.Identifier = ASCIItoUint12(&USBRxBuffer[1U+offset]);

			dlc = ASCIItoUint4(&USBRxBuffer[4U+offset]);
			CANTxHeader.DataLength = (dlc);

			if (USBRxBuffer[0+offset] == 'r')
			{
				CANTxHeader.TxFrameType = FDCAN_REMOTE_FRAME;
				// For remote frames, no need to copy CANTxData.
				dlc = 0U;
			}

			if ((commandLength - offset) == (5 + 2*DLCtoUINT8(dlc)))
			{
				uint8_t i = 0U;

				offset += 5U;
				while (offset < (commandLength-1) )
				{
					CANTxData[i++] = ASCIItoUint8(&USBRxBuffer[offset]);
					offset += 2U;
				}

				// Hold USB TX lock across ack + CAN send to prevent CAN RX from interleaving
				RAMN_USB_AcquireLock();
				RAMN_USB_SendFromTask_Locked((uint8_t*)"\r",1U);
				if (RAMN_FDCAN_SendMessage(&CANTxHeader,CANTxData) == RAMN_TRY_LATER)
				{
					// Release lock before retry loop to avoid blocking CAN RX forwarding
					RAMN_USB_ReleaseLock();
					while (RAMN_FDCAN_SendMessage(&CANTxHeader,CANTxData) == RAMN_TRY_LATER)
					{
						// Buffer is Full, Try later
						osDelay(10U);
					}
				}
				else
				{
					RAMN_USB_ReleaseLock();
				}

#if defined(CAN_ECHO)
				RAMN_USB_SendFromTask(USBRxBuffer,commandLength);
#endif

#if defined(PROCESS_SLCAN_BY_DBC)
				RAMN_DBC_ProcessCANMessage(CANTxHeader.Identifier,DLCtoUINT8(dlc),(RAMN_CANFrameData_t*)CANTxData);
#endif
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
		}
		else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
	}
	else if (((USBRxBuffer[0U+offset] == 'R') || (USBRxBuffer[0U+offset] == 'T')))
	{
		// 'T' : Transmit Extended ID DATA
		// 'R' : Transmit Extended ID RTR
		if ( (commandLength - offset) >= 10U)
		{
			CANTxHeader.IdType = FDCAN_EXTENDED_ID;
			CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
			CANTxHeader.Identifier =  ASCIItoUint32(&USBRxBuffer[1U+offset]);
			dlc = ASCIItoUint4(&USBRxBuffer[9U+offset]);
			CANTxHeader.DataLength = ((dlc));

			if (USBRxBuffer[0+offset] == 'R')
			{
				CANTxHeader.TxFrameType = FDCAN_REMOTE_FRAME;
				// For remote frames, no need to copy payload
				dlc = 0;
			}

			if ((commandLength - offset) == (10 + 2*DLCtoUINT8(dlc)))
			{

				offset += 10U;
				uint8_t i = 0U;
				while (offset < (commandLength-1) )
				{
					CANTxData[i++] = ASCIItoUint8(&USBRxBuffer[offset]);
					offset += 2U;
				}

				// Hold USB TX lock across ack + CAN send to prevent CAN RX from interleaving
				RAMN_USB_AcquireLock();
				RAMN_USB_SendFromTask_Locked((uint8_t*)"\r",1U);
				if (RAMN_FDCAN_SendMessage(&CANTxHeader,CANTxData) == RAMN_TRY_LATER)
				{
					// Release lock before retry loop to avoid blocking CAN RX forwarding
					RAMN_USB_ReleaseLock();
					while (RAMN_FDCAN_SendMessage(&CANTxHeader,CANTxData) == RAMN_TRY_LATER) osDelay(10U);
				}
				else
				{
					RAMN_USB_ReleaseLock();
				}
#if defined(CAN_ECHO)
				RAMN_USB_SendFromTask(USBRxBuffer,commandLength);
#endif

#if defined(PROCESS_SLCAN_BY_DBC)
				RAMN_DBC_ProcessCANMessage(CANTxHeader.Identifier,DLCtoUINT8(dlc),(RAMN_CANFrameData_t*)CANTxData);
#endif
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
		}
		else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
	}
	else
	{
		switch(USBRxBuffer[0U]){
		case 'O': // Open the channel
			RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
			RAMN_USB_Config.slcanOpened = True;
			RAMN_FDCAN_ResetPeripheral();
			break;
		case 'C': // Close the channel
			RAMN_USB_Config.slcanOpened = False;
			RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			break;
		case 'L': // Open in listening mode
			RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			hfdcan1.Init.Mode = FDCAN_MODE_BUS_MONITORING;
			RAMN_USB_Config.slcanOpened = True;
			RAMN_FDCAN_ResetPeripheral();
			break;
		case 'V': // Return SW version
			RAMN_USB_SendFromTask((uint8_t*)"V1 SLCAN RAMN (",15U);
			RAMN_USB_SendFromTask((uint8_t*)__DATE__,sizeof(__DATE__));
			RAMN_USB_SendFromTask((uint8_t*)" ",1U);
			RAMN_USB_SendFromTask((uint8_t*)__TIME__,sizeof(__TIME__));
			RAMN_USB_SendFromTask((uint8_t*)")\r",2U);
			break;
		case 'N': // Return serial number
			smallResponseBuffer[0U] = 'N';
			for(uint8_t k = 0; k <12U; k++)
			{
				uint8toASCII(*((uint8_t*)(HARDWARE_UNIQUE_ID_ADDRESS+k)),&smallResponseBuffer[1U+2*k]);
			}
			RAMN_USB_SendFromTask(smallResponseBuffer, 25U);
			RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			break;
		case 'S': // Set baudrate
			if (commandLength > 1U && commandLength <= 8U)
			{
				uint32_t baudrate = 0;
				RAMN_Bool_t valid = True;

				// Convert ASCII string to uint32_t
				for (uint8_t i = 1U; i < commandLength; i++)
				{
					if (USBRxBuffer[i] < '0' || USBRxBuffer[i] > '9') valid = False;
					baudrate = (baudrate * 10U) + (USBRxBuffer[i] - '0');
					if (baudrate > FDCAN_BITRATE_MAX) valid = False;
				}

				if(valid == True)
				{
					if (RAMN_FDCAN_UpdateBaudrate(baudrate) == RAMN_OK)  RAMN_USB_SendFromTask((uint8_t*)"\r", 1U);
					else RAMN_USB_SendFromTask((uint8_t*)"\a", 1U);
				}
				else RAMN_USB_SendFromTask((uint8_t*)"\a", 1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a", 1U);
			break;
		case 's':
			if (commandLength == 5U)
			{
				hfdcan1.Init.NominalPrescaler = 1; // Reset the prescaler in case it was modified by another function.
				hfdcan1.Init.NominalTimeSeg1 = ASCIItoUint8(&USBRxBuffer[1U]);
				hfdcan1.Init.NominalTimeSeg2 = ASCIItoUint8(&USBRxBuffer[3U]);
				RAMN_FDCAN_ResetPeripheral();
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
		case 'F':
			smallResponseBuffer[0U] = USBRxBuffer[0U];
			uint8toASCII(RAMN_FDCAN_Status.slcanFlags,&smallResponseBuffer[1U]);
			smallResponseBuffer[3U] = '\r';
			RAMN_USB_SendFromTask(smallResponseBuffer,4U);
			break;
		case 'W': // Set filter mode
			if (commandLength == 2U)
			{
				switch (USBRxBuffer[1U])
				{
				case '0':
					RAMN_FDCAN_Status.sFilterStdConfig.FilterType = FDCAN_FILTER_RANGE;
					RAMN_FDCAN_Status.sFilterExtConfig.FilterType = FDCAN_FILTER_RANGE;
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case '1':
					RAMN_FDCAN_Status.sFilterStdConfig.FilterType = FDCAN_FILTER_DUAL;
					RAMN_FDCAN_Status.sFilterExtConfig.FilterType = FDCAN_FILTER_DUAL;
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case '2':
					RAMN_FDCAN_Status.sFilterStdConfig.FilterType = FDCAN_FILTER_MASK; //Single Filter mask (Classic filter)
					RAMN_FDCAN_Status.sFilterExtConfig.FilterType = FDCAN_FILTER_MASK; //Single Filter mask (Classic filter)
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case '3':
					RAMN_FDCAN_Status.sFilterStdConfig.FilterType = FDCAN_FILTER_RANGE_NO_EIDM;
					RAMN_FDCAN_Status.sFilterExtConfig.FilterType = FDCAN_FILTER_RANGE_NO_EIDM;
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				default:
					RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
					break;
				}
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
		case 'M': // Set "Acceptance Code" Register (filter)
			if (commandLength == 4U)
			{
				RAMN_FDCAN_Status.sFilterStdConfig.FilterID1 = ASCIItoUint12(&USBRxBuffer[1])&0x7FF;
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else if (commandLength == 9U)
			{
				RAMN_FDCAN_Status.sFilterExtConfig.FilterID1 = ASCIItoUint32(&USBRxBuffer[1])&0x7FFFFFFF;
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
		case 'm': // Set "Acceptance Mask" Register (mask)
			if (commandLength == 4U)
			{
				RAMN_FDCAN_Status.sFilterStdConfig.FilterID2 = ASCIItoUint12(&USBRxBuffer[1])&0x7FF;
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else if (commandLength == 9U)
			{
				RAMN_FDCAN_Status.sFilterExtConfig.FilterID2 = ASCIItoUint32(&USBRxBuffer[1])&0x7FFFFFFF;
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
		case 'Z': // Enable time stamp
			if (commandLength == 2U)
			{
				if (USBRxBuffer[1U] == '0') RAMN_USB_Config.slcan_enableTimestamp = False;
				else RAMN_USB_Config.slcan_enableTimestamp = True;
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;

			/* BELOW ARE RAMN SPECIFIC COMMANDS */

		case 'w': // Update CAN controller settings
			RAMN_FDCAN_ResetPeripheral();
			RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			break;
		case 'i': // Enable/Disable ISO mode
			if (commandLength == 2U)
			{
				if (USBRxBuffer[1U] == '0') HAL_FDCAN_DisableISOMode(&hfdcan1);
				else HAL_FDCAN_EnableISOMode(&hfdcan1);
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
			//				case 'e': //Enable/Disable edge filtering
			//					if (USBRxBuffer[1U] == '0') HAL_FDCAN_DisableEdgeFiltering(&hfdcan1);
			//					else HAL_FDCAN_EnableEdgeFiltering(&hfdcan1);
			//					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			//					break;
			//				case 'g': //Enable/Disable TX Compensation
			//					if (USBRxBuffer[1U] == '0') HAL_FDCAN_DisableTxDelayCompensation(&hfdcan1);
			//					else HAL_FDCAN_EnableTxDelayCompensation(&hfdcan1);
			//					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			//					break;
		case 'G': // Set Nominal and Data SJW
			if ((commandLength == 3U) || (commandLength == 5U))
			{
				hfdcan1.Init.NominalSyncJumpWidth = ASCIItoUint8(&USBRxBuffer[1U]);
				if(commandLength == 5U)
				{
					hfdcan1.Init.DataSyncJumpWidth = ASCIItoUint8(&USBRxBuffer[3U]);
				}
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else
			{
				RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			}

			break;
		case 'a': // Enable/Disable TX auto retransmission
			if (commandLength == 2U)
			{
				if (USBRxBuffer[1U] == '0') hfdcan1.Init.AutoRetransmission = DISABLE;
				else hfdcan1.Init.AutoRetransmission = ENABLE;
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
		case 'f': // Select Frame mode
			if (commandLength == 2U)
			{
				if (USBRxBuffer[1U] == '0') hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
				else if (USBRxBuffer[1U] == '1') hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
				else hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_BRS;
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else
			{
				RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			}
			break;
		case 'v': // Add a "i" to frames with the ESI flag set
			if (commandLength == 2U)
			{
				if (USBRxBuffer[1U] == '0') RAMN_USB_Config.addESIFlag = False;
				else RAMN_USB_Config.addESIFlag = True;
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else
			{
				RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			}
			break;
#ifndef HARDENING
		case '@': //Select auto report of errors
			if (commandLength == 2U)
			{
				if (USBRxBuffer[1U] == '0') RAMN_USB_Config.autoreportErrors = False;
				else if (USBRxBuffer[1U] == '1') RAMN_USB_Config.autoreportErrors = True;
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
#endif
#ifndef HARDENING // Remove this check to reenable CLI mode in Hardening.
		case '#': //Enable CLI
			if (commandLength == 1U)
			{
				mustSwitch = True;
				RAMN_USB_SendStringFromTask("Welcome to RAMN CLI. Type 'help' for help.\r>");
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
#endif
#if defined(ENABLE_USB) && defined(ENABLE_USB_DEBUG)
		case 'd': // Enable/Disable debug reports
			if (commandLength == 2U)
			{
				if (USBRxBuffer[1U] == '0') RAMN_DEBUG_SetStatus(False);
				else if (USBRxBuffer[1U] == '1')RAMN_DEBUG_SetStatus(True);
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
#endif
		case 'k': // Configure "Nominal (arbitration) phase" bit rate
			if (commandLength == 9U)
			{
				hfdcan1.Init.NominalPrescaler = ASCIItoUint16(&USBRxBuffer[1]); //16-bit prescaler
				hfdcan1.Init.NominalTimeSeg1 = ASCIItoUint8(&USBRxBuffer[5]);
				hfdcan1.Init.NominalTimeSeg2 = ASCIItoUint8(&USBRxBuffer[7]);
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
		case 'K': // Configure "Data phase" bit rate
			if (commandLength == 7U)
			{
				hfdcan1.Init.DataPrescaler = ASCIItoUint8(&USBRxBuffer[1]); //8-bit prescaler
				hfdcan1.Init.DataTimeSeg1 = ASCIItoUint8(&USBRxBuffer[3]);
				hfdcan1.Init.DataTimeSeg2 = ASCIItoUint8(&USBRxBuffer[5]);
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
		case 'j': // Return Random byte
			smallResponseBuffer[0U] = USBRxBuffer[0U];
			uint8toASCII(RAMN_RNG_Pop8()&0xFF,&smallResponseBuffer[1U]);
			smallResponseBuffer[3U] = '\r';
			RAMN_USB_SendFromTask(smallResponseBuffer,4U);
			break;
		case 'J': // Return Random Integer
			smallResponseBuffer[0U] = USBRxBuffer[0U];
			uint32toASCII(RAMN_RNG_Pop32(),&smallResponseBuffer[1]);
			smallResponseBuffer[9U] = '\r';
			RAMN_USB_SendFromTask(smallResponseBuffer,10U);
			break;
		case '?':
		case 'h':
		case 'H':
			RAMN_USB_SendFromTask((uint8_t*)"https://ramn.rtfd.io/\r",22U);
			break;
		case 'l': // Open in restricted mode
			RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			hfdcan1.Init.Mode = FDCAN_MODE_RESTRICTED_OPERATION;
			RAMN_USB_Config.slcanOpened = True;
			RAMN_FDCAN_ResetPeripheral();
			break;
		case 'E':// Full Error and protocol flags
			HAL_FDCAN_GetProtocolStatus(&hfdcan1,&protocolStatus);
			HAL_FDCAN_GetErrorCounters(&hfdcan1, &errorCount);
#ifdef ENABLE_USB_DEBUG
			RAMN_DEBUG_DumpCANErrorRegisters(&errorCount, &protocolStatus);
#endif
			break;
		case 'q': // Get status of FIFOs
			// Reports the status of each Stream Buffer over USB, stores data in provided buffer
			RAMN_USB_SendStringFromTask("q");

			// RX FIFO Fill Level
			RAMN_USB_SendASCIIUint32(HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1,FDCAN_RX_FIFO0));

			// TX FIFO Free Level
			RAMN_USB_SendASCIIUint32(HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1));

#ifdef ENABLE_CDC
			// CAN RX Stream Buffer levels
			RAMN_USB_SendASCIIUint32(xStreamBufferSpacesAvailable(CANRxDataStreamBufferHandle));
			RAMN_USB_SendASCIIUint32(xStreamBufferBytesAvailable(CANRxDataStreamBufferHandle));

			// CAN TX Stream Buffer levels
			RAMN_USB_SendASCIIUint32(xStreamBufferSpacesAvailable(CANTxDataStreamBufferHandle));
			RAMN_USB_SendASCIIUint32(xStreamBufferBytesAvailable(CANTxDataStreamBufferHandle));
#endif
			RAMN_USB_SendStringFromTask("\r");
			break;
#ifdef ENABLE_USB_DEBUG
		case 'I':// Send GW Stats information
			RAMN_DEBUG_ReportCANStats(&RAMN_FDCAN_Status);
			break;
#endif
		case 'D':// Restart in DFU Mode
			// Note that we do not worry about potential timing analysis as this is not a security feature.
			if (commandLength == (RAMN_strlen(DFU_COMMAND_STRING)+1U))
			{
				if(strncmp((char*)&USBRxBuffer[1],DFU_COMMAND_STRING, RAMN_strlen(DFU_COMMAND_STRING)) == 0U)
				{
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					osDelay(200);
					if(RAMN_FLASH_isMemoryProtected() == False) RAMN_FLASH_ConfigureOptionBytesBootloaderMode();
					else RAMN_FLASH_RemoveMemoryProtection();
				}
			}
			// Board should reset automatically, if we reach here there was an error
			RAMN_USB_SendFromTask((uint8_t*)"\a",1);
			break;
		case 'p':// Program ECU over CAN
			if (commandLength == 2U)
			{
				if( (USBRxBuffer[1U] == 'B') || (USBRxBuffer[1U] == 'C') || (USBRxBuffer[1U] == 'D'))
				{
					RAMN_USB_Config.simulatorActive = False; //Turn off simulator
					RAMN_USB_Config.slcanOpened = False; //Turn off slcan to prevent forwarding bootloader test commands
					CANTxHeader.BitRateSwitch = FDCAN_BRS_ON;
					CANTxHeader.DataLength = 0U;
					CANTxHeader.FDFormat = FDCAN_FD_CAN;
					CANTxHeader.IdType = FDCAN_STANDARD_ID;
					CANTxHeader.Identifier = 0x181;
					for(uint8_t j = 0; j < BOOTLOADER_MAX_ATTEMPTS; j++)
					{
						RAMN_ECU_SetEnableAll(0U);
						RAMN_FDCAN_SetupForSTBootloader();
						RAMN_FDCAN_ResetPeripheral();
						osDelay(100U); // wait for power supply disable to be effective
						RAMN_ECU_SetBoot0(USBRxBuffer[1U],GPIO_PIN_SET);
						RAMN_ECU_SetEnable(USBRxBuffer[1U],GPIO_PIN_SET);
						osDelay(20U + j*20U); //add increasingly long delay if bootloader transition failed
						while (RAMN_FDCAN_SendMessage(&CANTxHeader,CANTxData) == RAMN_TRY_LATER)
						{
							//Buffer is Full, Try later
							osDelay(10U);
						}
						osDelay(50U); //leave time to send message and receive responses
						if (RAMN_FDCAN_Status.CANRXCnt > 0U)
						{
							//received an answer, assume it is from bootloader
							break;
						}
					}
					if (RAMN_FDCAN_Status.CANRXCnt == 0U)
					{
						RAMN_USB_SendFromTask((uint8_t*)"\a",1U); //all bootloader transitions failed
					}
					else
					{
						RAMN_USB_Config.slcanOpened = True; //make sure slcan is opened for following commands
						RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					}
				}
				else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
		case 'Y': // Set ENABLE of all ECUs
			if (commandLength == 2U)
			{
				if (USBRxBuffer[1U] == '0') RAMN_ECU_SetEnableAll(GPIO_PIN_RESET);
				else RAMN_ECU_SetEnableAll(GPIO_PIN_SET);
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
		case 'y': // Set ENABLE of one ECU
			if (commandLength == 3U)
			{
				if( (USBRxBuffer[1U] == 'B') || (USBRxBuffer[1U] == 'C') || (USBRxBuffer[1U] == 'D'))
				{
					if (USBRxBuffer[2U] == '0') RAMN_ECU_SetEnable(USBRxBuffer[1U],GPIO_PIN_RESET);
					else RAMN_ECU_SetEnable(USBRxBuffer[1U],GPIO_PIN_SET);
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
				}
				else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break;
		case 'n':// Reset whole board (used to leave programming mode)
			if (commandLength == 1U)
			{
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
#ifdef ENABLE_USB_DEBUG
				RAMN_DEBUG_Log("d Resetting\r");
#endif
				RAMN_ECU_SetEnableAll(GPIO_PIN_RESET);
				RAMN_ECU_SetBoot0All(GPIO_PIN_RESET);
				osDelay(100);
				HAL_NVIC_SystemReset();
			} else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			break; // Should not reach here
		case 'c': // Connection to computer
			if (commandLength == 2U)
			{
				if (USBRxBuffer[1U] == '0')
				{
					RAMN_DBC_RequestSilence = True;
					RAMN_USB_Config.simulatorActive = False;
				}
				else
				{
					hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
					RAMN_USB_Config.slcanOpened = False; //close slcan mode by default
					RAMN_FDCAN_ResetPeripheral(); //reset in case settings have changed or port has never been opened
					RAMN_USB_Config.simulatorActive = True;
					RAMN_DBC_RequestSilence = False;
				}
				RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);

			break;
#ifdef GENERATE_RUNTIME_STATS
		case 'X': // Display FreeRTOS stats
			if (uxTaskGetNumberOfTasks() > MAX_NUMBER_OF_TASKS) RAMN_USB_SendStringFromTask("Too many tasks\r");
			else
			{
				unsigned long ulTotalRunTime;
				uint8_t usage;

				RAMN_USB_SendStringFromTask("Stats computed from boot state. Use rightmost values to compute deltas.\r");
				uxTaskGetSystemState(pxTaskStatusArray, uxTaskGetNumberOfTasks(), &ulTotalRunTime);
				RAMN_USB_SendStringFromTask("No\tTask Name\tState\tUsage\tStack\t");
				uintToBCD(ulTotalRunTime, (char*)smallResponseBuffer);
				RAMN_USB_SendStringFromTask((char*)smallResponseBuffer);
				RAMN_USB_SendStringFromTask(" (ulTotalRunTime)\r");

				for (uint8_t i = 0; i < uxTaskGetNumberOfTasks(); i++)
				{
					if (ulTotalRunTime > 0U) usage =  (100U * pxTaskStatusArray[i].ulRunTimeCounter) / ulTotalRunTime;
					else usage = 0U;



					uintToBCD(i, (char*)smallResponseBuffer);
					RAMN_USB_SendStringFromTask((char*)smallResponseBuffer);
					RAMN_USB_SendStringFromTask(":\t");
					RAMN_USB_SendStringFromTask(pxTaskStatusArray[i].pcTaskName);
					if(strlen(pxTaskStatusArray[i].pcTaskName) < 8) RAMN_USB_SendStringFromTask("\t\t");
					else RAMN_USB_SendStringFromTask("\t");

					// Send task state
					const char *stateStr = "?????";
					switch(pxTaskStatusArray[i].eCurrentState) {
					case eRunning:   stateStr = "Running"; break;
					case eReady:     stateStr = "Ready"; break;
					case eBlocked:   stateStr = "Block"; break;
					case eSuspended: stateStr = "Suspend"; break;
					case eDeleted:   stateStr = "Deleted"; break;
					case eInvalid:   stateStr = "Invalid"; break;
					}
					RAMN_USB_SendStringFromTask(stateStr);

					// Send Usage
					uintToBCD(usage, (char*)smallResponseBuffer);
					RAMN_USB_SendStringFromTask("\t");
					RAMN_USB_SendStringFromTask((char*)smallResponseBuffer);
					RAMN_USB_SendStringFromTask("%");

					// Convert stack high water mark to a string and send
					uintToBCD(pxTaskStatusArray[i].usStackHighWaterMark, (char*)smallResponseBuffer);
					RAMN_USB_SendStringFromTask("\t");
					RAMN_USB_SendStringFromTask((char*)smallResponseBuffer);

					RAMN_USB_SendStringFromTask("\t");
					uintToBCD(pxTaskStatusArray[i].ulRunTimeCounter, (char*)smallResponseBuffer);
					RAMN_USB_SendStringFromTask((char*)smallResponseBuffer);
					RAMN_USB_SendStringFromTask("\r");
				}
			}
			break;
#endif
#ifndef HARDENING
		case 'x': //Get Microcontroller Unique ID Address
			smallResponseBuffer[0U] = USBRxBuffer[0U];
			uint32toASCII((uint32_t)*(HARDWARE_UNIQUE_ID_ADDRESS),&smallResponseBuffer[1]);
			smallResponseBuffer[9U] = '\r';
			RAMN_USB_SendFromTask(smallResponseBuffer,10U);
			break;
#endif
#ifdef ENABLE_UDS
//#ifndef HARDENING   // Uncomment this to disable UDS over USB when HARDENING flag is active.
		case '%': // Diagnostic Message
			if (commandLength >= 4U)
			{
				uint16_t reqSize = (commandLength-4)/2; // Remove command byte and payload size
				if (reqSize == ASCIItoUint12(&USBRxBuffer[1U]))
				{
					uint16_t ansSize;
					ASCIItoRaw(diagRxUSBbuf,&USBRxBuffer[4U],reqSize);
					RAMN_UDS_ProcessDiagPayload(xTaskGetTickCount(), diagRxUSBbuf, reqSize, diagTxUSBbuf, &ansSize);
					// We do not need the USB RX buffer anymore, so we use it so save the answer
					uint12toASCII(ansSize, &USBRxBuffer[1U]);
					rawtoASCII(&USBRxBuffer[4U],diagTxUSBbuf,ansSize);
					USBRxBuffer[ansSize*2U+4U] = '\r';
					// Add 1 for %, 1 for \r
					RAMN_USB_SendFromTask(USBRxBuffer,(ansSize*2U)+5U);
					RAMN_UDS_PerformPostAnswerActions(xTaskGetTickCount(), diagRxUSBbuf, reqSize, diagTxUSBbuf, &ansSize);
				}
				else RAMN_USB_SendFromTask((uint8_t*)"\a",1);
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1);
			break;
//#endif
#endif
		case 'b': // Already in slcan mode, just say yes
			RAMN_USB_SendFromTask((uint8_t*)"\r",1);
			break;
#if defined(ENABLE_MINICTF) && defined(TARGET_ECUA)
		case '^':
			RAMN_USB_SendStringFromTask(FLAG_USB_1);
			RAMN_USB_SendFromTask((uint8_t*)"\r",1);
			break;
		case '&':
			if (commandLength == 6)
			{
				if (memcmp(&USBRxBuffer[1],"27762",5) == 0)
				{
					RAMN_USB_SendStringFromTask(FLAG_USB_2);
					RAMN_USB_SendFromTask((uint8_t*)"\r",1);
				}
				else RAMN_USB_SendStringFromTask("Wrong Password\r");
			}
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1);
			break;
#endif
		case 'P':
		case 'A':
		case 'U':
		case 'Q':
		default:
			RAMN_USB_SendFromTask((uint8_t*)"\a",1);
			break;
		}
	}
	return mustSwitch;
}

#endif
