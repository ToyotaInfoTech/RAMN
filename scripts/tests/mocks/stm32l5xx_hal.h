#ifndef __MOCK_STM32L5XX_HAL_H
#define __MOCK_STM32L5XX_HAL_H

#include "main.h"

// FDCAN types
typedef struct {
    uint32_t Instance;
} FDCAN_HandleTypeDef;

typedef struct {
    uint32_t FilterType;
    uint32_t FilterConfig;
    uint32_t FilterID1;
    uint32_t FilterID2;
} FDCAN_FilterTypeDef;

extern FDCAN_HandleTypeDef hfdcan1;

// Callbacks
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);
void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes);
void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan);
void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs);

// CRC types
typedef struct {
    uint32_t Instance;
} CRC_HandleTypeDef;

// RNG types
typedef struct {
    uint32_t Instance;
} RNG_HandleTypeDef;

// GPIO
#define GPIO_PIN_0 0
#define GPIOA NULL

#endif
