#ifndef __MOCK_MAIN_H
#define __MOCK_MAIN_H

#include <stdint.h>
#include <stddef.h>

// Basic RAMN types
typedef enum {
    RAMN_OK = 0,
    RAMN_ERROR = 1,
    RAMN_BUSY = 2,
    RAMN_TIMEOUT = 3,
    RAMN_TRY_LATER = 2
} RAMN_Result_t;

typedef enum {
    False = 0,
    True = 1
} RAMN_Bool_t;

// FDCAN definitions
typedef struct {
    uint32_t Identifier;
    uint32_t IdType;
    uint32_t TxFrameType;
    uint32_t DataLength;
    uint32_t ErrorStateIndicator;
    uint32_t BitRateSwitch;
    uint32_t FDFormat;
    uint32_t TxEventFifoControl;
    uint32_t MessageMarker;
} FDCAN_TxHeaderTypeDef;

typedef struct {
    uint32_t Identifier;
    uint32_t IdType;
    uint32_t RxFrameType;
    uint32_t DataLength;
    uint32_t ErrorStateIndicator;
    uint32_t BitRateSwitch;
    uint32_t FDFormat;
    uint32_t IsFilterMatchingFrame;
    uint32_t FilterIndex;
} FDCAN_RxHeaderTypeDef;

#define FDCAN_STANDARD_ID   0x00000000U
#define FDCAN_EXTENDED_ID   0x00000001U
#define FDCAN_DATA_FRAME    0x00000000U
#define FDCAN_ESI_ACTIVE    0x00000000U
#define FDCAN_BRS_OFF       0x00000000U
#define FDCAN_CLASSIC_CAN   0x00000000U
#define FDCAN_NO_TX_EVENTS  0x00000000U
#define FDCAN_DLC_BYTES_0   0
#define FDCAN_DLC_BYTES_8   8

// ISO-TP Config
#define ISOTP_RXBUFFER_SIZE 4096
#define ISOTP_TXBUFFER_SIZE 4096
#define ISOTP_RX_TIMEOUT_MS 1000
#define ISOTP_TX_TIMEOUT_MS 1000
#define ISOTP_CONSECUTIVE_BLOCK_SIZE 0
#define ISOTP_CONSECUTIVE_ST 0

#define ISOTP_RX 0x1
#define ISOTP_TX 0x2

typedef enum {
	N_OK, N_TIMEOUT_A, N_TIMEOUT_Bs, N_TIMEOUT_Cr, N_WRONG_SN,
	N_INVALID_FS, N_UNEXP_PDU, N_WFT_OVRN, N_BUFFER_OVFLW, N_ERROR,
} RAMN_ISOTP_N_RESULT;

typedef enum {
  ISOTP_ERROR_NOERROR,
  ISOTP_ERROR_TIMEOUT,
  ISOTP_ERROR_INVALID_DATA,
  ISOTP_ERROR_SEQUENCE,
  ISOTP_ERROR_TARGET_ABORT,
  ISOTP_ERROR_OVERFLOW,
} RAMN_ISOTP_ErrorCode_t ;

typedef enum {
	ISOTP_RX_IDLE, ISOTP_RX_TRANSFERRING, ISOTP_RX_FINISHED,
} RAMN_ISOTP_RXStatus_t;

typedef enum {
  ISOTP_TX_IDLE, ISOTP_TX_TRANSFERRING, ISOTP_TX_WAITING_FLAG, ISOTP_TX_FINISHED,
} RAMN_ISOTP_TXStatus_t ;

typedef struct {
	RAMN_ISOTP_RXStatus_t rxStatus;
	uint16_t rxCount;
	uint16_t rxExpectedSize;
	uint32_t rxLastTimestamp;
	uint8_t  rxMustSendCF;
	uint8_t  rxData[ISOTP_RXBUFFER_SIZE];
	uint16_t rxFrameCount;
	RAMN_ISOTP_TXStatus_t txStatus;
	uint16_t txIndex;
	uint16_t txSize;
	uint32_t txLastTimestamp;
	uint8_t  txData[ISOTP_TXBUFFER_SIZE];
	uint16_t txFrameCount;
	uint8_t selfFCFlag;
	uint8_t selfBlockSize;
	uint8_t selfST;
	uint8_t targetFCFlag;
	uint8_t targetBlockSize;
	uint8_t targetST;
	FDCAN_TxHeaderTypeDef* pFC_CANHeader;
} RAMN_ISOTPHandler_t;

// ISO-TP Prototypes
void RAMN_ISOTP_Init(RAMN_ISOTPHandler_t* handler, FDCAN_TxHeaderTypeDef* FCMsgHeader);
void RAMN_ISOTP_ProcessRxMsg(RAMN_ISOTPHandler_t* handler, uint8_t dlc, const uint8_t* data, const uint32_t tick);
RAMN_Bool_t RAMN_ISOTP_GetFCFrame(RAMN_ISOTPHandler_t* handler, uint8_t* dlc, uint8_t* data);
RAMN_Bool_t RAMN_ISOTP_GetNextTxMsg(RAMN_ISOTPHandler_t* handler, uint8_t* dlc, uint8_t* data, uint32_t tick);
RAMN_Result_t RAMN_ISOTP_Update(RAMN_ISOTPHandler_t* pHandler, uint32_t tick);
RAMN_Bool_t RAMN_ISOTP_Continue_TX(RAMN_ISOTPHandler_t* pHandler, uint32_t tick, FDCAN_TxHeaderTypeDef* pTxHeader);
RAMN_Result_t RAMN_ISOTP_RequestTx(RAMN_ISOTPHandler_t* handler, uint32_t tick);

// Mock helper functions
uint8_t DLCtoUINT8(uint32_t dlc);
uint32_t UINT8toDLC(uint8_t size);

// External modules dependencies
typedef volatile struct
{
	volatile uint16_t control_brake; 			//CANID_CONTROL_BRAKE
	volatile uint16_t command_brake; 			//CANID_COMMAND_BRAKE
	volatile uint16_t control_accel; 			//CANID_CONTROL_ACCEL
	volatile uint16_t command_accel; 			//CANID_COMMAND_ACCEL
	volatile uint16_t control_steer; 			//CANID_CONTROL_STEERING
	volatile uint16_t command_steer; 			//CANID_COMMAND_STEERING
	volatile uint8_t  control_shift;			//CANID_CONTROL_SHIFT
	volatile uint16_t command_shift;			//CANID_COMMAND_SHIFT
	volatile uint16_t control_sidebrake; 		//CANID_CONTROL_SIDEBRAKE
	volatile uint16_t command_sidebrake; 		//CANID_COMMAND_SIDEBRAKE
	volatile uint16_t status_rpm; 				//CANID_STATUS_RPM
	volatile uint8_t command_horn;				//CANID_COMMAND_HORN
	volatile uint8_t control_horn; 				//CANID_CONTROL_HORN
	volatile uint16_t command_lights; 			//CANID_COMMAND_LIGHTS
	volatile uint16_t command_turnindicator;	//CANID_COMMAND_TURNINDICATOR
	volatile uint16_t control_enginekey;		//CANID_CONTROL_ENGINEKEY
	volatile uint16_t control_lights; 			//CANID_CONTROL_LIGHTS
	volatile uint8_t joystick;					// Second byte of SHIFT, saved for convenience

} RAMN_DBC_Handle_t;
extern RAMN_DBC_Handle_t RAMN_DBC_Handle;
extern uint8_t RAMN_DBC_RequestSilence;

// Mock callbacks for Python
typedef void (*tx_callback_t)(FDCAN_TxHeaderTypeDef*, const uint8_t*);
void set_tx_callback(tx_callback_t cb);

// FreeRTOS & HAL Stubs
void osDelay(uint32_t ticks);
void HAL_NVIC_SystemReset(void);
void Error_Handler(void);

// Diag Constants
#define UDS_MAXIMUM_RPM_ACCEPTABLE 5000
#define UDS_SESSION_TIMEOUT_MS 5000
#define SECURITY_ACCESS_MAX_ATTEMPTS 5
#define SECURITY_ACCESS_RETRY_TIMEOUT_MS 1000

// Target IDs (proxies for what's in ramn_vehicle_specific.h)
#include "ramn_j1939.h"

#if defined(TARGET_ECUA)
#define UDS_RX_CANID 0x7E0
#define KWP_RX_CANID 0x7E0
#define XCP_RX_CANID 0x7E0
#define J1939_ECU_SA J1939_SA_HEADWAY_CTRL
#elif defined(TARGET_ECUB)
#define UDS_RX_CANID 0x7E1
#define KWP_RX_CANID 0x7E1
#define XCP_RX_CANID 0x7E1
#define J1939_ECU_SA J1939_SA_STEERING_CTRL
#elif defined(TARGET_ECUC)
#define UDS_RX_CANID 0x7E2
#define KWP_RX_CANID 0x7E2
#define XCP_RX_CANID 0x7E2
#define J1939_ECU_SA J1939_SA_POWERTRAIN_CTRL
#elif defined(TARGET_ECUD)
#define UDS_RX_CANID 0x7E3
#define KWP_RX_CANID 0x7E3
#define XCP_RX_CANID 0x7E3
#define J1939_ECU_SA J1939_SA_BODY_CTRL
#endif

#define UDS_TX_CANID (UDS_RX_CANID + 8)
#define KWP_TX_CANID (KWP_RX_CANID + 8)
#define XCP_TX_CANID (XCP_RX_CANID + 8)

// Functional IDs
#define UDS_FUNCTIONAL_RX_CANID 0x7DF

// NRCs
#define UDS_NRC_VSTH 0x88
#define UDS_NRC_GPF  0x72

// ARM intrinsics stubs
#define __DSB()
#define __ISB()

// Handle types
typedef struct { uint32_t i; } CRC_HandleTypeDef;
typedef struct { uint32_t i; } RNG_HandleTypeDef;
typedef struct { uint32_t Instance; } TIM_HandleTypeDef;

// J1939 Routing Macro
#define J1939_UCAST_ID(prio, pgn, da, sa) \
      ((((uint32_t)(prio) & 0x7) << 26) | \
       (((uint32_t)(pgn) & 0xFF00) << 8) | \
       (((uint32_t)(da) & 0xFF) << 8) | \
       ((uint32_t)(sa) & 0xFF))

// DBC Frame Data mock
typedef union
{
	uint8_t rawData[8];
} RAMN_CANFrameData_t;

// Missing prototypes for Diag
uint32_t RAMN_CRC_SoftCalculate(const uint8_t* data, uint32_t size);
void RAMN_FDCAN_Disable(void);
RAMN_Result_t RAMN_FDCAN_UpdateBaudrate(uint32_t b);
void RAMN_FDCAN_ResetPeripheral(void);
RAMN_Result_t RAMN_FDCAN_UpdateTiming(uint8_t* d);
RAMN_Result_t RAMN_FDCAN_UpdateSJW(uint8_t s);
RAMN_Result_t RAMN_FDCAN_UpdateSettings(uint8_t* d);
RAMN_Result_t RAMN_FLASH_ConfigureOptionBytesBootloaderMode(void);
RAMN_Result_t RAMN_FLASH_RemoveMemoryProtection(void);
RAMN_Result_t RAMN_FDCAN_SendMessage(const FDCAN_TxHeaderTypeDef* header, const uint8_t* data);

// Init Prototypes
RAMN_Result_t RAMN_UDS_Init(uint32_t tick);
RAMN_Result_t RAMN_KWP_Init(uint32_t tick);
RAMN_Result_t RAMN_XCP_Init(uint32_t tick);

// Payload Prototypes
void RAMN_UDS_ProcessDiagPayload(uint32_t tick, uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);
void RAMN_UDS_ProcessDiagPayloadFunctional(uint32_t tick, uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);
void RAMN_KWP_ProcessDiagPayload(uint32_t tick, const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);
void RAMN_XCP_ProcessDiagPayload(uint32_t tick, const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);

// Memory & Flash Prototypes
RAMN_Bool_t RAMN_MEMORY_CheckAreaReadable(uint32_t start, uint32_t end);
RAMN_Bool_t RAMN_RAM_CheckAreaWritable(uint32_t start, uint32_t end);
RAMN_Bool_t RAMN_FLASH_CheckFlashAreaValidForFirmware(uint32_t start, uint32_t end);
RAMN_Bool_t RAMN_FLASH_isMemoryProtected(void);
RAMN_Result_t RAMN_FLASH_EraseAlternativeFirmware(void);
RAMN_Result_t RAMN_FLASH_SwitchActiveBank(void);
RAMN_Result_t RAMN_FLASH_Write64(uint32_t address, uint64_t data);

// J1979 Proto
void RAMN_J1979_ProcessMessage(const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);

// memcpy/memset
void RAMN_memcpy(void* dst, const void* src, uint32_t size);
void RAMN_memset(void* dst, uint8_t byte, uint32_t size);
uint16_t RAMN_strlen(const char *str);

// StreamBuffer stub
#include "stream_buffer.h"

// Diag ProcessRx Prototypes
RAMN_Bool_t RAMN_UDS_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick, StreamBufferHandle_t* strbuf);
RAMN_Bool_t RAMN_KWP_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick, StreamBufferHandle_t* strbuf);
RAMN_Bool_t RAMN_XCP_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick, StreamBufferHandle_t* strbuf);

// Simulator & Screen Prototypes
extern uint8_t RAMN_SIM_AutopilotEnabled;
RAMN_Result_t RAMN_SIM_SetBrake(uint16_t b);
RAMN_Result_t RAMN_SIM_SetAccel(uint16_t a);
RAMN_Result_t RAMN_SIM_SetSteering(int16_t s);
RAMN_Result_t RAMN_SIM_SetShift(uint8_t s);
RAMN_Result_t RAMN_SIM_SetHorn(uint8_t h);
RAMN_Result_t RAMN_SIM_SetLights(uint8_t l);
RAMN_Result_t RAMN_SIM_SetTurnIndicator(uint16_t t);
RAMN_Result_t RAMN_SIM_SetEngineKey(uint8_t k);
RAMN_Result_t RAMN_SIM_SetSidebrake(uint16_t s);
void RAMN_SCREEN_Refresh(void);
void RAMN_SCREEN_SetForceRefresh(uint8_t f);

// XCP device name
#define XCP_DEVICE_NAME "MOCK"

// Diag loop continue protos
RAMN_Bool_t RAMN_UDS_Continue_TX(uint32_t tick);
RAMN_Bool_t RAMN_KWP_Continue_TX(uint32_t tick);
RAMN_Bool_t RAMN_XCP_Continue_TX(uint32_t tick, const uint8_t* data, uint16_t size);

#endif
