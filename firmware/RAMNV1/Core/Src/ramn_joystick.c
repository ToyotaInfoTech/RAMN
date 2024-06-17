/*
 * ramn_joystick.c
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
 ******************************************************************************
 */

#include "ramn_joystick.h"

//Stream buffer holding joystick events
volatile static StreamBufferHandle_t joystickStreamBufferHandle;

////Semaphore to enable access to this module from different threads
static SemaphoreHandle_t JOYSTICK_SEMAPHORE;
static StaticSemaphore_t JOYSTICK_SEMAPHORE_STRUCT;
static StaticStreamBuffer_t JOYSTICK_POOL_STRUCT;

//Static buffer that holds data from the stream buffer
static uint8_t JOYSTICK_POOL[JOYSTICK_POOL_SIZE];

static uint8_t previous_joystick_data = 0U;

// Exported features -----------------------------

void RAMN_Joystick_Init()
{
	joystickStreamBufferHandle    = xStreamBufferCreateStatic(JOYSTICK_POOL_SIZE,sizeof(uint8_t),JOYSTICK_POOL,&JOYSTICK_POOL_STRUCT);
	JOYSTICK_SEMAPHORE   = xSemaphoreCreateMutexStatic(&JOYSTICK_SEMAPHORE_STRUCT);
}

//must be called with RAMN_DBC_Handle.control_shift >> 8
void RAMN_Joystick_Update(uint8_t joystick_data)
{

	if (joystickStreamBufferHandle == 0) return; //not initialized yet

	if (joystick_data != previous_joystick_data)
	{
		//joystick action detected
		JoystickEventType action = JOYSTICK_EVENT_NONE;
		switch(joystick_data)
		{
		case 0x02:
			//up
			action = JOYSTICK_EVENT_UP_PRESSED;
			break;
		case 0x03:
			//down
			action = JOYSTICK_EVENT_DOWN_PRESSED;
			break;
		case 0x04:
			//right
			action = JOYSTICK_EVENT_RIGHT_PRESSED;
			break;
		case 0x05:
			//left
			action = JOYSTICK_EVENT_LEFT_PRESSED;
			break;
		case 0x06:
			//center
			action = JOYSTICK_EVENT_CENTER_PRESSED;
			break;
		case 0x01:
			//button was released
			switch (previous_joystick_data)
			{
			case 0x02:
				//up
				action = JOYSTICK_EVENT_UP_RELEASED;
				break;
			case 0x03:
				//down
				action = JOYSTICK_EVENT_DOWN_RELEASED;
				break;
			case 0x04:
				//right
				action = JOYSTICK_EVENT_RIGHT_RELEASED;
				break;
			case 0x05:
				//left
				action = JOYSTICK_EVENT_LEFT_RELEASED;
				break;
			case 0x06:
				//center
				action = JOYSTICK_EVENT_CENTER_RELEASED;
				break;
			default:
				action = JOYSTICK_EVENT_INVALID;
				break;
			}
			break;
		default:
			action = JOYSTICK_EVENT_INVALID;
			break;
		}

		if (action != JOYSTICK_EVENT_NONE)
		{
			if(xStreamBufferSpacesAvailable(joystickStreamBufferHandle) > 0)
			{
				xStreamBufferSend(joystickStreamBufferHandle, (void *)&action, sizeof(action),0U);
			}
		}
	}

	previous_joystick_data = joystick_data;


}

JoystickEventType RAMN_Joystick_Pop(void)
{
	JoystickEventType result;
	while (xSemaphoreTake(JOYSTICK_SEMAPHORE, portMAX_DELAY ) != pdTRUE);
	if (xStreamBufferBytesAvailable(joystickStreamBufferHandle) >= sizeof(result))
	{
	xStreamBufferReceive(joystickStreamBufferHandle,(void *)&result,sizeof(result),portMAX_DELAY);
	}
	else
	{
		result = JOYSTICK_EVENT_NONE;
	}
	xSemaphoreGive(JOYSTICK_SEMAPHORE);
	return result;
}

