#include "main.h"
#include "cmsis_os.h"
#include "stream_buffer.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// Global state mocks
RAMN_DBC_Handle_t RAMN_DBC_Handle = { .status_rpm = 0.0f };
uint8_t RAMN_DBC_RequestSilence = 0;

// Mock StreamBuffer handles and pointers to handles
static uint32_t uds_buffer_obj = 0x11111111;
static uint32_t kwp_buffer_obj = 0x22222222;
static uint32_t xcp_buffer_obj = 0x33333333;

static StreamBufferHandle_t uds_handle = (StreamBufferHandle_t)&uds_buffer_obj;
static StreamBufferHandle_t kwp_handle = (StreamBufferHandle_t)&kwp_buffer_obj;
static StreamBufferHandle_t xcp_handle = (StreamBufferHandle_t)&xcp_buffer_obj;

static StreamBufferHandle_t* uds_handle_ptr = &uds_handle;
static StreamBufferHandle_t* kwp_handle_ptr = &kwp_handle;
static StreamBufferHandle_t* xcp_handle_ptr = &xcp_handle;

StreamBufferHandle_t* get_uds_handle_ptr(void) { return uds_handle_ptr; }
StreamBufferHandle_t* get_kwp_handle_ptr(void) { return kwp_handle_ptr; }
StreamBufferHandle_t* get_xcp_handle_ptr(void) { return xcp_handle_ptr; }

// Dummy handle for backward compatibility
StreamBufferHandle_t* get_dummy_handle_ptr(void) {
    return uds_handle_ptr;
}

// Callback mechanism
static tx_callback_t current_tx_callback = NULL;

void set_tx_callback(tx_callback_t cb) {
    current_tx_callback = cb;
}

// HAL & FreeRTOS Mocks
RAMN_Result_t RAMN_FDCAN_SendMessage(const FDCAN_TxHeaderTypeDef* header, const uint8_t* data) {
    if (current_tx_callback) {
        current_tx_callback((FDCAN_TxHeaderTypeDef*)header, data);
    }
    return RAMN_OK;
}

void osDelay(uint32_t ticks) {}
void RAMN_TaskDelay(uint32_t ticks) {}
uint32_t xTaskNotifyGive(osThreadId_t task) { return 0; }

// Dummy buffers for diag payloads
static uint8_t uds_ans_buf[4096];
static uint16_t uds_ans_size;
static uint8_t kwp_ans_buf[4096];
static uint16_t kwp_ans_size;
static uint8_t xcp_ans_buf[4096];
static uint16_t xcp_ans_size;

size_t xStreamBufferSend(StreamBufferHandle_t xStreamBuffer, const void *pvTxData, size_t xDataLengthBytes, uint32_t xTicksToWait) {
    // Smart Mock: Detect which protocol is sending data to the buffer and trigger response
    static uint8_t uds_functional = 0;
    static uint16_t uds_expected = 0;
    static uint16_t uds_received = 0;
    static uint8_t uds_temp_payload[4096];

    static uint16_t kwp_expected = 0;
    static uint16_t kwp_received = 0;
    static uint8_t kwp_temp_payload[4096];

    static uint16_t xcp_expected = 0;
    static uint16_t xcp_received = 0;
    static uint8_t xcp_temp_payload[4096];

    static int uds_state = 0; // 0: wait functional, 1: wait size, 2: wait payload

    uint8_t* data = (uint8_t*)pvTxData;

    if (xStreamBuffer == uds_handle) {
        if (xDataLengthBytes == 1 && uds_state == 0) {
            uds_functional = data[0];
            uds_state = 1;
        } else if (xDataLengthBytes == 2 && uds_state == 1) {
            uds_expected = *(uint16_t*)data;
            uds_received = 0;
            uds_state = 2;
        } else if (uds_state == 2) {
            memcpy(&uds_temp_payload[uds_received], data, xDataLengthBytes);
            uds_received += xDataLengthBytes;
            if (uds_received >= uds_expected) {
                uint8_t ans_buf[4096];
                uint16_t ans_size = 0;
                if (uds_functional) {
                    RAMN_UDS_ProcessDiagPayloadFunctional(0, uds_temp_payload, uds_expected, ans_buf, &ans_size);
                } else {
                    RAMN_UDS_ProcessDiagPayload(0, uds_temp_payload, uds_expected, ans_buf, &ans_size);
                }
                if (ans_size > 0) {
                    extern RAMN_ISOTPHandler_t RAMN_UDS_ISOTPHandler;
                    extern FDCAN_TxHeaderTypeDef udsFCMsgHeader;
                    RAMN_ISOTP_Init(&RAMN_UDS_ISOTPHandler, &udsFCMsgHeader);
                    RAMN_memcpy(RAMN_UDS_ISOTPHandler.txData, ans_buf, ans_size);
                    RAMN_UDS_ISOTPHandler.txSize = ans_size;
                    RAMN_UDS_ISOTPHandler.txStatus = ISOTP_TX_TRANSFERRING;
                    RAMN_UDS_Continue_TX(0);
                }
                uds_expected = 0;
                uds_state = 0;
            }
        }
    } else if (xStreamBuffer == kwp_handle) {
        if (xDataLengthBytes == sizeof(uint16_t) && kwp_expected == 0) {
            kwp_expected = *(uint16_t*)data;
            kwp_received = 0;
        } else if (kwp_expected > 0) {
            memcpy(&kwp_temp_payload[kwp_received], data, xDataLengthBytes);
            kwp_received += xDataLengthBytes;
            if (kwp_received >= kwp_expected) {
                uint8_t ans_buf[4096];
                uint16_t ans_size = 0;
                RAMN_KWP_ProcessDiagPayload(0, kwp_temp_payload, kwp_expected, ans_buf, &ans_size);
                if (ans_size > 0) {
                    extern RAMN_ISOTPHandler_t RAMN_KWP_ISOTPHandler;
                    extern FDCAN_TxHeaderTypeDef kwpFCMsgHeader;
                    RAMN_ISOTP_Init(&RAMN_KWP_ISOTPHandler, &kwpFCMsgHeader);
                    RAMN_memcpy(RAMN_KWP_ISOTPHandler.txData, ans_buf, ans_size);
                    RAMN_KWP_ISOTPHandler.txSize = ans_size;
                    RAMN_KWP_ISOTPHandler.txStatus = ISOTP_TX_TRANSFERRING;
                    RAMN_KWP_Continue_TX(0);
                }
                kwp_expected = 0;
            }
        }
    } else if (xStreamBuffer == xcp_handle) {
        if (xDataLengthBytes == sizeof(uint16_t) && xcp_expected == 0) {
            xcp_expected = *(uint16_t*)data;
            xcp_received = 0;
        } else if (xcp_expected > 0) {
            memcpy(&xcp_temp_payload[xcp_received], data, xDataLengthBytes);
            xcp_received += xDataLengthBytes;
            if (xcp_received >= xcp_expected) {
                uint8_t ans_buf[4096];
                uint16_t ans_size = 0;
                RAMN_XCP_ProcessDiagPayload(0, xcp_temp_payload, xcp_expected, ans_buf, &ans_size);
                if (ans_size > 0) {
                    extern FDCAN_TxHeaderTypeDef RAMN_XCP_TxMsgHeader;
                    RAMN_XCP_TxMsgHeader.DataLength = UINT8toDLC((uint8_t)ans_size);
                    RAMN_FDCAN_SendMessage(&RAMN_XCP_TxMsgHeader, ans_buf);
                }
                xcp_expected = 0;
            }
        }
    }
    return xDataLengthBytes;
}

// Peripherals Mocks
uint32_t RAMN_RNG_Pop32(void) {
    return 0x12345678;
}

void Error_Handler(void) {
}

void HAL_NVIC_SystemReset(void) {
    fprintf(stderr, "SystemReset requested!\n");
}

// Memory & Flash Mocks
RAMN_Bool_t RAMN_MEMORY_CheckAreaReadable(uint32_t start, uint32_t end) { return True; }
RAMN_Bool_t RAMN_RAM_CheckAreaWritable(uint32_t start, uint32_t end) { return True; }
RAMN_Bool_t RAMN_FLASH_CheckFlashAreaValidForFirmware(uint32_t start, uint32_t end) { return True; }
RAMN_Bool_t RAMN_FLASH_isMemoryProtected(void) { return False; }
RAMN_Result_t RAMN_FLASH_EraseAlternativeFirmware(void) { return RAMN_OK; }
RAMN_Result_t RAMN_FLASH_SwitchActiveBank(void) { return RAMN_OK; }
RAMN_Result_t RAMN_FLASH_Write64(uint32_t address, uint64_t data) { return RAMN_OK; }
RAMN_Result_t RAMN_FLASH_ConfigureOptionBytesBootloaderMode(void) { return RAMN_OK; }
RAMN_Result_t RAMN_FLASH_RemoveMemoryProtection(void) { return RAMN_OK; }

// Utils Mocks
void RAMN_memcpy(void* dst, const void* src, uint32_t size) {
    // fprintf(stderr, "  RAMN_memcpy: dst=%p, src=%p, size=%d\n", dst, src, size);
    memcpy(dst, src, size);
}

void RAMN_memset(void* dst, uint8_t byte, uint32_t size) {
    // fprintf(stderr, "  RAMN_memset: dst=%p, byte=0x%02x, size=%d\n", dst, byte, size);
    memset(dst, byte, size);
}

uint16_t RAMN_strlen(const char *str) {
    return (uint16_t)strlen(str);
}

// DLC Helpers
uint8_t DLCtoUINT8(uint32_t dlc_enum) {
    if (dlc_enum <= 8) return (uint8_t)dlc_enum;
    return 8;
}

uint32_t UINT8toDLC(uint8_t size) {
    return (uint32_t)size;
}

// Mock DTC
RAMN_Result_t RAMN_DTC_Init(void) { return RAMN_OK; }
RAMN_Result_t RAMN_DTC_ClearAll(void) { return RAMN_OK; }
RAMN_Result_t RAMN_DTC_GetNumberOfDTC(uint32_t *n) { *n = 0; return RAMN_OK; }

// Diag missing mocks
uint32_t RAMN_CRC_SoftCalculate(const uint8_t* data, uint32_t size) { return 0xDEADBEEF; }
void RAMN_FDCAN_Disable(void) {}
RAMN_Result_t RAMN_FDCAN_UpdateBaudrate(uint32_t b) { return RAMN_OK; }
void RAMN_FDCAN_ResetPeripheral(void) {}
RAMN_Result_t RAMN_FDCAN_UpdateTiming(uint8_t* d) { return RAMN_OK; }
RAMN_Result_t RAMN_FDCAN_UpdateSJW(uint8_t s) { return RAMN_OK; }
RAMN_Result_t RAMN_FDCAN_UpdateSettings(uint8_t* d) { return RAMN_OK; }

// J1979 Mock
void RAMN_J1979_ProcessMessage(const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize) {
    if (size > 0) {
        answerData[0] = data[0] + 0x40;
        *answerSize = 1;
    }
}

// Simulator Mocks
uint8_t RAMN_SIM_AutopilotEnabled = 0;
RAMN_Result_t RAMN_SIM_SetBrake(uint16_t b) { return RAMN_OK; }
RAMN_Result_t RAMN_SIM_SetAccel(uint16_t a) { return RAMN_OK; }
RAMN_Result_t RAMN_SIM_SetSteering(int16_t s) { return RAMN_OK; }
RAMN_Result_t RAMN_SIM_SetShift(uint8_t s) { return RAMN_OK; }
RAMN_Result_t RAMN_SIM_SetHorn(uint8_t h) { return RAMN_OK; }
RAMN_Result_t RAMN_SIM_SetLights(uint8_t l) { return RAMN_OK; }
RAMN_Result_t RAMN_SIM_SetTurnIndicator(uint16_t t) { return RAMN_OK; }
RAMN_Result_t RAMN_SIM_SetEngineKey(uint8_t k) { return RAMN_OK; }
RAMN_Result_t RAMN_SIM_SetSidebrake(uint16_t s) { return RAMN_OK; }

// Screen Manager Mock
void RAMN_SCREEN_Refresh(void) {}
void RAMN_SCREEN_SetForceRefresh(uint8_t f) {}

void vTaskDelete(void* handle) {}
