#ifndef __MOCK_STREAM_BUFFER_H
#define __MOCK_STREAM_BUFFER_H

#include <stdint.h>
#include <stddef.h>

typedef void* StreamBufferHandle_t;

size_t xStreamBufferSend(StreamBufferHandle_t xStreamBuffer, const void *pvTxData, size_t xDataLengthBytes, uint32_t xTicksToWait);

#endif
