/*
 * ramn_ctf.h
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

// This Module is to implement small CTF training challenges
#ifndef INC_RAMN_CTF_H_
#define INC_RAMN_CTF_H_

#include "main.h"
#include "ramn_canfd.h"
#include "ramn_utils.h"

#ifdef ENABLE_MINICTF

#define FLAG_USB_1 "flag{USB_COMMAND}"
#define FLAG_USB_2 "flag{USB_BRUTEFORCE}"
#define FLAG_CAN_1 "flag{CAN_FIRST_MESSAGE}"
#define FLAG_CAN_2 "flag{CAN_RTR_ANSWER}"
#define FLAG_CAN_3 "flag{CAN_FLAG_GET}"
#define FLAG_CAN_4 "flag{CAN_BINARY_ANALYSIS}"
#define FLAG_CAN_5 "flag{ONE_BY_ONE}"
#define FLAG_CAN_6 "flag{IN_CAN_FLAGS}"
#define FLAG_CAN_7 "flag{IN_CAN_TIMING}"
#define FLAG_UDS_1 "flag{UDS_DATA_READ}"
#define FLAG_UDS_2 "flag{UDS_ENUMERATION}"
#define FLAG_UDS_3 "flag{HIDDEN_SERVICE}"
#define FLAG_UDS_4 "flag{READ_MEMORY}"
#define FLAG_UDS_5 "flag{AUTHENTICATED}"






//Function to initializes the CTF handler
void 	RAMN_CTF_Init(uint32_t tick);

//Process an incoming CAN Message
void	RAMN_CTF_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick);

//Updates the module. Should be called periodically.
void 	RAMN_CTF_Update(uint32_t tick);


#endif

#endif /* INC_RAMN_CTF_H_ */
