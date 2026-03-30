#ifndef __MOCK_CMSIS_OS_H
#define __MOCK_CMSIS_OS_H

#include <stdint.h>

typedef void* osThreadId_t;
#define portMAX_DELAY 0xFFFFFFFFUL

void osDelay(uint32_t ticks);
uint32_t xTaskNotifyGive(osThreadId_t task);

// Stub for RAMN_TaskDelay
void RAMN_TaskDelay(uint32_t ticks);

#endif
