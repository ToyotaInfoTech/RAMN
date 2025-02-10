/*
 * ramn_screen_canmonitor.c
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
 */

#include "ramn_screen_canmonitor.h"

#ifdef ENABLE_SCREEN

// Semaphore to enable access  from different threads
static SemaphoreHandle_t CANMONITOR_SEMAPHORE = 0U;
static StaticSemaphore_t CANMONITOR_SEMAPHORE_STRUCT;

// Number of new messages to display
volatile static uint8_t newMsgCnt = 0U;

// Number of identifiers observed
volatile static uint8_t idCnt = 0U;

// Flag set when a new identifier was added and a screen redraw is needed
volatile static RAMN_Bool_t newIdentifierAdded = False;

// Flag set when an identifier cannot be displayed on screen.
volatile static RAMN_Bool_t identifierOverflowed = False;

// Linked list for storing messages
static CANMessageNode* head = NULL;

// Ticks of last update
static uint32_t lastUpdated = 0;

static void freeCANMessageList()
{
	CANMessageNode* current = head;
	CANMessageNode* next;

	while (current != NULL)
	{
		next = current->next;
		free(current);
		current = next;
	}
	idCnt = 0U;
	head = NULL;
}

//TODO: optimize access by keeping pointer to last item
static CANMessageNode* findOrCreateCANMessageNode(uint32_t identifier)
{
	CANMessageNode* current = head;
	CANMessageNode* previous = NULL;

	// Find the correct position or existing node
	while (current != NULL && current->identifier < identifier)
	{
		previous = current;
		current = current->next;
	}

	// Return if node exists
	if (current != NULL && current->identifier == identifier)  return current;

	if (idCnt >= MAX_CANMONITOR_IDS)
	{
		// Remove the last node to make space if the new identifier has higher priority
		CANMessageNode* temp = head;
		CANMessageNode* prevTemp = NULL;

		while (temp->next != NULL)
		{
			prevTemp = temp;
			temp = temp->next;
		}

		// Last item in the list
		if (temp != NULL)
		{
			if (temp->identifier > identifier) // Last message has lower priority (higher identifier)
			{
				free(temp);
				if (prevTemp != NULL) prevTemp->next = NULL;
			}
			else return NULL; // No place for this new identifier
		}
		idCnt--;
		identifierOverflowed = True;
	}

	// Create new node
	CANMessageNode* newNode = (CANMessageNode*)malloc(sizeof(CANMessageNode));

	if (newNode == NULL)
	{
		// No place in memory
		identifierOverflowed = True;
		return NULL;
	}
	newNode->identifier = identifier;
	RAMN_memset(newNode->messages, 0, sizeof(newNode->messages));
	RAMN_memset(newNode->lastNibbleChange, 0, sizeof(newNode->lastNibbleChange));
	newNode->next = current;

	if (previous == NULL) head = newNode;
	else previous->next = newNode;

	if (newNode->next == newNode) newNode->next = NULL; // Case where we are at the end of the list

	idCnt++;
	newIdentifierAdded = True;
	return newNode;
}

// Removes messages with old timestamps
uint8_t removeOldNodes(uint32_t thresholdTick)
{
	CANMessageNode* current = head;
	CANMessageNode* previous = NULL;
	uint8_t node_deleted = 0U;

	while (current != NULL)
	{
		// Check if the current node's latest message timestamp is less than the threshold
		if (current->messages[0].header.RxTimestamp < thresholdTick)
		{
			CANMessageNode* nodeToRemove = current;

			// If the node to remove is the head of the list
			if (previous == NULL) head = current->next;
			else previous->next = current->next;

			current = current->next;
			free(nodeToRemove);
			idCnt--;
			node_deleted = 1U;
		} else {
			previous = current;
			current = current->next;
		}
	}
	return node_deleted;
}

static void SCREENCANMONITOR_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick)
{
	//TODO: process more types (?)
	if ((pHeader->IdType == FDCAN_STANDARD_ID) && (pHeader->FDFormat == FDCAN_CLASSIC_CAN) && (pHeader->RxFrameType == FDCAN_DATA_FRAME) && (pHeader->DataLength <= 8))
	{

		if (CANMONITOR_SEMAPHORE == 0U) return; // Module not initialized yet, skip message.
		while (xSemaphoreTake(CANMONITOR_SEMAPHORE, portMAX_DELAY ) != pdTRUE);

		// Process Message
		CANMessageNode* node = findOrCreateCANMessageNode(pHeader->Identifier);
		if (node == NULL) identifierOverflowed = True;
		else
		{
			node->messages[1] = node->messages[0];
			node->messages[0].header = *pHeader;
			node->messages[0].header.RxTimestamp = tick;

			RAMN_memcpy(node->messages[0].data, data, pHeader->DataLength);

			for(uint8_t i=0; i < pHeader->DataLength; i++)
			{
				if ((node->messages[0].data[i]&0xF) != (node->messages[1].data[i]&0xF)) node->lastNibbleChange[(2*i)+1] = tick;
				if ((node->messages[0].data[i]&0xF0) != (node->messages[1].data[i]&0xF0)) node->lastNibbleChange[(2*i)] = tick;
			}
			newMsgCnt++;
		}
		xSemaphoreGive(CANMONITOR_SEMAPHORE);
	}
}

static void SCREENCANMONITOR_Init()
{
	if (CANMONITOR_SEMAPHORE == 0U) CANMONITOR_SEMAPHORE   = xSemaphoreCreateMutexStatic(&CANMONITOR_SEMAPHORE_STRUCT);
	RAMN_SCREENUTILS_DrawBase();
	newIdentifierAdded = True; // Make sure the IDs get drawn
	RAMN_SPI_DrawString(42,5, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "CAN RX MONITOR");
}

static void SCREENCANMONITOR_Update(uint32_t tick)
{
	uint8_t msgCnt = 0;
	uint8_t tmp[21];

	if (RAMN_SCREENUTILS_LoopCounter % 5U == 0U)
	{
		if ((newMsgCnt > 0) || (tick - lastUpdated > 500U))
		{
			while (xSemaphoreTake(CANMONITOR_SEMAPHORE, portMAX_DELAY ) != pdTRUE);

			if (tick > 2000U)
			{
				if (removeOldNodes(tick - 2000U) != 0U)
				{
					// Screen redraw needed pretend there is a new ID
					RAMN_SPI_DrawRectangle(5,25+((idCnt)*16),LCD_WIDTH-10,(MAX_CANMONITOR_IDS-idCnt)*16,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND);
					newIdentifierAdded = 1U;
				}
			}

			CANMessageNode* current = head;
			if (newIdentifierAdded != False)
			{
				newIdentifierAdded = False;

				if (identifierOverflowed != False)
				{
					RAMN_SPI_DrawString(200,5, RAMN_SCREENUTILS_COLORTHEME.WHITE, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, "OVF");
					identifierOverflowed = False; // No need to redisplay message
				}

				// Update IDs

				while (current != NULL)
				{
					uint12toASCII(current->identifier, tmp);
					tmp[3] = 0;
					RAMN_SPI_DrawString(7,25+(msgCnt*16), RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, (char*)tmp);
					current = current->next;
					msgCnt += 1;
				}
			}

			current = head; // Reset head location
			msgCnt = 0;
			while (current != NULL)
			{
				for(uint8_t i=0;i<current->messages[0].header.DataLength*2;i++)
				{
					uint4toASCII((current->messages[0].data[i/2] >> (4*((i+1)%2)))&0xF,&tmp[i]);

					if (tick - current->lastNibbleChange[i] >= 500U)
					{
						RAMN_SPI_RefreshChar(7+(4*11)+i*11,25+(msgCnt*16), RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, tmp[i]);
					}
					else
					{
						RAMN_SPI_RefreshChar(7+(4*11)+i*11,25+(msgCnt*16), RAMN_SCREENUTILS_COLORTHEME.WHITE, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, tmp[i]);
					}
				}
				if (8 - current->messages[0].header.DataLength > 0)
				{
					// Erase bytes after DLC (that could have been written by a previous message with longer DLC)
					RAMN_SPI_DrawRectangle(7+(4*11)+current->messages[0].header.DataLength*22,25+(msgCnt*16),22*(8 - current->messages[0].header.DataLength),14,RAMN_SCREENUTILS_COLORTHEME.BACKGROUND);
				}
				current = current->next;
				msgCnt += 1;
			}
			newMsgCnt = 0U;
			lastUpdated = tick;
			xSemaphoreGive(CANMONITOR_SEMAPHORE);
		}
		RAMN_SCREENUTILS_DrawSubconsoleUpdate();
	}
}

static void SCREENCANMONITOR_Deinit()
{
	freeCANMessageList();
}

RAMNScreen ScreenCANMonitor = {
		.Init = SCREENCANMONITOR_Init,
		.Update = SCREENCANMONITOR_Update,
		.Deinit = SCREENCANMONITOR_Deinit,
		.UpdateInput = 0U,
		.ProcessRxCANMessage = SCREENCANMONITOR_ProcessRxCANMessage
};

#endif
