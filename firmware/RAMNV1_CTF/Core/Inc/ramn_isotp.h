/*
 * ramn_isotp.h
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

// This module implements the ISO-TP Layer of UDS/KWP (ISO 15765)

#ifndef INC_RAMN_ISOTP_H_
#define INC_RAMN_ISOTP_H_

#include "main.h"

#if defined(ENABLE_ISOTP)

#include "ramn_canfd.h"

//Possible Error Codes for both TX and RX
typedef enum {
  ISOTP_ERROR_NOERROR,         //No error
  ISOTP_ERROR_TIMEOUT,    	   //timeout
  ISOTP_ERROR_INVALID_DATA,    //invalid data received
  ISOTP_ERROR_SEQUENCE,        //invalid sequence
  ISOTP_ERROR_TARGET_ABORT,    //target asked for cancellation
  ISOTP_ERROR_OVERFLOW,        //target asked for transfer too large
} RAMN_ISOTP_ErrorCode_t ;

//RX Status
typedef enum {
	ISOTP_RX_IDLE,
	ISOTP_RX_TRANSFERRING,
	ISOTP_RX_FINISHED,
} RAMN_ISOTP_RXStatus_t;

//TX Status
typedef enum {
  ISOTP_TX_IDLE,               //no processing
  ISOTP_TX_TRANSFERRING,       //sending/receiving data
  ISOTP_TX_WAITING_FLAG,       //waiting for a "CF" flag, TX only
  ISOTP_TX_FINISHED,
} RAMN_ISOTP_TXStatus_t ;

//Error Codes as defined by ISO Standard
typedef enum {
	N_OK,	//success
	N_TIMEOUT_A,
	N_TIMEOUT_Bs,
	N_TIMEOUT_Cr,
	N_WRONG_SN,
	N_INVALID_FS,
	N_UNEXP_PDU,
	N_WFT_OVRN,
	N_BUFFER_OVFLW,
	N_ERROR,
} RAMN_ISOTP_N_RESULT;

//This structure keeps the status of current ISO-TP Communication
typedef struct
{
	RAMN_ISOTP_RXStatus_t rxStatus;  		//Current Status of RX Communication
	uint16_t rxCount;						//Number of Bytes received (current frame)
	uint16_t rxExpectedSize;				//Total Number of bytes expected for current communication
	uint32_t rxLastTimestamp;		    	//Timestamp of last received message
	uint8_t  rxMustSendCF;					//Target is expecting a CF
	uint8_t  rxData[ISOTP_RXBUFFER_SIZE];	//Buffer for Outgoing Data
	uint16_t rxFrameCount;  				//frame count (current number of CAN Messages received, not whole ISO-TP frames received)

	RAMN_ISOTP_TXStatus_t txStatus;    		//Current Status of TX Communication
	uint16_t txIndex;				   		//Number of bytes currently sent
	uint16_t txSize;				   		//Number of bytes to send in total
	uint32_t txLastTimestamp;		   		//Timestamp of last TX request (Not actual time sent)
	uint8_t txData[ISOTP_TXBUFFER_SIZE];	//Buffer for Incoming Data
	uint16_t txFrameCount;  				//frame count PER ISO-TP message (not per session)

	uint8_t selfFCFlag;				   		//FC Flag reported by self
	uint8_t selfBlockSize;			   		//Block Size requested by self
	uint8_t selfST;					   		//ST (Time between frames) requested by self

	uint8_t targetFCFlag;			  		//FC Flag reported by target
	uint8_t targetBlockSize;		  		//Block Size requested by target
	uint8_t targetST;						//ST (Time between frames) requested by target

	FDCAN_TxHeaderTypeDef* pFC_CANHeader;	//Header of the CAN Message to use for Flow Control Frames

} RAMN_ISOTPHandler_t;


//This Function initializes the specified ISO-TP Handler
void 			RAMN_ISOTP_Init(RAMN_ISOTPHandler_t* handler, FDCAN_TxHeaderTypeDef* FCMsgHeader);

//This Function adds a message for process by the ISO TP Engine. It assumes CAN ID has already been checked
void		 	RAMN_ISOTP_ProcessRxMsg(RAMN_ISOTPHandler_t* handler, uint8_t dlc, const uint8_t* data, const uint32_t tick);

//This function formats the next "FC Frame" CAN Message to be sent next. User must call this function periodically and send the message if it returns True.
RAMN_Bool_t 	RAMN_ISOTP_GetFCFrame(RAMN_ISOTPHandler_t* handler, uint8_t* dlc, uint8_t* data);

//This function formats the next CAN Message to be sent (except FC Frames). User must call this function periodically and send the message if it returns True.
RAMN_Bool_t 	RAMN_ISOTP_GetNextTxMsg(RAMN_ISOTPHandler_t* handler, uint8_t* dlc, uint8_t* data, uint32_t tick);

//This function Updates the ISO-TP Engine, and must be called periodically.
RAMN_Result_t 	RAMN_ISOTP_Update(RAMN_ISOTPHandler_t* pHandler, uint32_t tick);

//This function updates the TX part of the ISO-TP Engine. Must be called periodically when sending.
RAMN_Bool_t 	RAMN_ISOTP_Continue_TX(RAMN_ISOTPHandler_t* pHandler, uint32_t tick, FDCAN_TxHeaderTypeDef* pTxHeader);

//This function request the ISO-TP Engine to process an outgoing message.
RAMN_Result_t 	RAMN_ISOTP_RequestTx(RAMN_ISOTPHandler_t* handler, uint32_t tick);

#endif

#endif /* INC_RAMN_ISOTP_H_ */
