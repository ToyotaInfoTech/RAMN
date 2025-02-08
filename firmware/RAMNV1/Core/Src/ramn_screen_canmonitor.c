/*
 * ramn_screen_canmonitor.c
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
 */

#include "ramn_screen_canmonitor.h"

#ifdef ENABLE_SCREEN


////Semaphore to enable access  from different threads
static SemaphoreHandle_t CANMONITOR_SEMAPHORE = 0U;
static StaticSemaphore_t CANMONITOR_SEMAPHORE_STRUCT;


//linked list for storing messages
CANMessageNode* head = NULL;
//number of new messages to display
volatile uint8_t new_messages = 0U;

//number of identifiers observed
volatile uint8_t identifierCount = 0U;
//flag set when a new identifier was added and a screen redraw is needed
volatile uint8_t newIdentifierAdded = 0U;
//flag set when an identifier cannot be displayed on screen.
volatile uint8_t identifierOverflowed = 0U;


static void freeCANMessageList() {
	CANMessageNode* current = head;
	CANMessageNode* next;

	while (current != NULL) {
		next = current->next;
		free(current);
		current = next;
	}
	identifierCount = 0U;
	head = NULL;
}

//TODO optimize access
static CANMessageNode* findOrCreateCANMessageNode(uint32_t identifier) {
	CANMessageNode* current = head;
	CANMessageNode* previous = NULL;

	//find the correct position or existing node
	while (current != NULL && current->identifier < identifier) {
		previous = current;
		current = current->next;
	}

	// return if node exists
	if (current != NULL && current->identifier == identifier) {
		return current;
	}

	if (identifierCount >= MAX_CANMONITOR_IDS) {

		// Remove the last node to make space if the new identifier has higher priority
		CANMessageNode* temp = head;
		CANMessageNode* prevTemp = NULL;
		while (temp->next != NULL) {
			prevTemp = temp;
			temp = temp->next;
		}

		//last item in the list
		if (temp != NULL) {

			if (temp->identifier > identifier) {
				free(temp);
				if (prevTemp != NULL) prevTemp->next = NULL;
			}
			else
			{
				return NULL; //no place for this new identifier
			}
		}
		identifierCount--;
		identifierOverflowed = 1;
	}

	CANMessageNode* newNode = (CANMessageNode*)malloc(sizeof(CANMessageNode));
	if (newNode == NULL) {
		identifierOverflowed = 1;
		return NULL;
	}
	newNode->identifier = identifier;
	memset(newNode->messages, 0, sizeof(newNode->messages));
	memset(newNode->lastNibbleChange, 0, sizeof(newNode->lastNibbleChange));
	newNode->next = current;

	if (previous == NULL) {
		head = newNode;
	} else {
		previous->next = newNode;
	}
	if (newNode->next == newNode) newNode->next = NULL; //case where we are at the end of the list

	identifierCount++;
	newIdentifierAdded = 1;

	return newNode;
}

//Function to remove messages with old timestamps
uint8_t removeOldNodes(uint32_t thresholdTick) {
	CANMessageNode* current = head;
	CANMessageNode* previous = NULL;
	uint8_t node_deleted = 0U;
	while (current != NULL) {
		// Check if the current node's latest message timestamp is less than the threshold
		if (current->messages[0].header.RxTimestamp < thresholdTick) {
			CANMessageNode* nodeToRemove = current;

			// If the node to remove is the head of the list
			if (previous == NULL) {
				head = current->next;
			} else {
				previous->next = current->next;
			}

			current = current->next;
			free(nodeToRemove);
			identifierCount--;
			node_deleted = 1U;
		} else {
			previous = current;
			current = current->next;
		}
	}
	return node_deleted;
}

static void ScreenCANMonitor_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick)
{
	//TODO process more types?
	if ((pHeader->IdType == FDCAN_STANDARD_ID) && (pHeader->FDFormat == FDCAN_CLASSIC_CAN) && (pHeader->RxFrameType == FDCAN_DATA_FRAME) && (pHeader->DataLength <= 8))
	{

		while (CANMONITOR_SEMAPHORE == 0U) osDelay(50);
		while (xSemaphoreTake(CANMONITOR_SEMAPHORE, portMAX_DELAY ) != pdTRUE);

		//Process Message here
		CANMessageNode* node = findOrCreateCANMessageNode(pHeader->Identifier);
		if (node == NULL) {
			identifierOverflowed = 1U;
		}
		else
		{
			node->messages[1] = node->messages[0];

			node->messages[0].header = *pHeader;
			node->messages[0].header.RxTimestamp = tick;

			RAMN_memcpy(node->messages[0].data, data, pHeader->DataLength);

			//TODO better handle DLC changes
			for(uint8_t i=0; i < pHeader->DataLength; i++)
			{
				if ((node->messages[0].data[i]&0xF) != (node->messages[1].data[i]&0xF))
				{
					node->lastNibbleChange[(2*i)+1] = tick;
				}
				if ((node->messages[0].data[i]&0xF0) != (node->messages[1].data[i]&0xF0))
				{
					node->lastNibbleChange[(2*i)] = tick;
				}
			}
			//end processing
			new_messages++;
		}
		xSemaphoreGive(CANMONITOR_SEMAPHORE);
	}
}

static void ScreenCANMonitor_Init() {
	if (CANMONITOR_SEMAPHORE == 0U) CANMONITOR_SEMAPHORE   = xSemaphoreCreateMutexStatic(&CANMONITOR_SEMAPHORE_STRUCT);
	RAMN_SCREENUTILS_DrawBase();
	newIdentifierAdded = 1U; // make sure the IDs get drawn
	RAMN_SPI_DrawString(42,5, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "CAN RX MONITOR");
}

uint32_t last_update = 0;
static void ScreenCANMonitor_Update(uint32_t tick) {

	uint8_t msg_cnt = 0;
	uint8_t tmp[21];

	if (RAMN_SCREENUTILS_LoopCounter % 5 == 0)
	{
		if ((new_messages > 0) || (tick - last_update > 500))
		{

			while (xSemaphoreTake(CANMONITOR_SEMAPHORE, portMAX_DELAY ) != pdTRUE);

			if (tick > 2000)
			{
				if (removeOldNodes(tick - 2000) != 0U)
				{
					//screen redraw needed pretend there is a new ID
					RAMN_SPI_DrawRectangle(5,25+((identifierCount)*16),LCD_WIDTH-10,(MAX_CANMONITOR_IDS-identifierCount)*16,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND);
					newIdentifierAdded = 1U;
				}
			}

			CANMessageNode* current = head;
			if (newIdentifierAdded != 0)
			{
				newIdentifierAdded = 0U;

				if (identifierOverflowed != 0U)
				{
					RAMN_SPI_DrawString(200,5, RAMN_SCREENUTILS_COLORTHEME.WHITE, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "OVF");
					identifierOverflowed = 0;
				}

				//Update IDs

				while (current != NULL) {

					uint12toASCII(current->identifier, tmp);
					tmp[3] = 0;
					RAMN_SPI_DrawString(7,25+(msg_cnt*16), RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, tmp);

					current = current->next;
					msg_cnt += 1;
				}


			}

			current = head; //reset head location
			msg_cnt = 0;
			while (current != NULL) {

				for(uint8_t i=0;i<current->messages[0].header.DataLength*2;i++)
				{
					uint4toASCII((current->messages[0].data[i/2] >> (4*((i+1)%2)))&0xF,&tmp[i]);

					if (tick - current->lastNibbleChange[i] >= 500)
					{
						RAMN_SPI_RefreshChar(7+(4*11)+i*11,25+(msg_cnt*16), RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, tmp[i]);
					}
					else
					{
						RAMN_SPI_RefreshChar(7+(4*11)+i*11,25+(msg_cnt*16), RAMN_SCREENUTILS_COLORTHEME.WHITE, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, tmp[i]);
					}
				}
				if (8 - current->messages[0].header.DataLength > 0)
				{
					RAMN_SPI_DrawRectangle(7+(4*11)+current->messages[0].header.DataLength*22,25+(msg_cnt*16),22*(8 - current->messages[0].header.DataLength),14,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND);
				}
				current = current->next;
				msg_cnt += 1;
			}
			new_messages = 0U;

			last_update = tick;
			xSemaphoreGive(CANMONITOR_SEMAPHORE);
		}

		RAMN_SCREENUTILS_DrawSubconsoleUpdate();
	}


}

static void ScreenCANMonitor_Deinit() {
	freeCANMessageList();
}

static RAMN_Bool_t ScreenCANMonitor_UpdateInput(JoystickEventType event) {
	return True;
}

RAMNScreen ScreenCANMonitor = {
		.Init = ScreenCANMonitor_Init,
		.Update = ScreenCANMonitor_Update,
		.Deinit = ScreenCANMonitor_Deinit,
		.UpdateInput = ScreenCANMonitor_UpdateInput,
		.ProcessRxCANMessage = ScreenCANMonitor_ProcessRxCANMessage
};

#endif
