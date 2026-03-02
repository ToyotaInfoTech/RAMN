/*
 * ramn_utils.c
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2025 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include "ramn_utils.h"
#include "cmsis_os.h"

// Mapping of ASCII characters to hex values
static const uint8_t ascii_hashmap[] =
{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  !"#$%&'
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ()*+,-./
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 01234567
		0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 89:;<=>?
		0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // @ABCDEFG
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // HIJKLMNO
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // PQRSTUVW
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // XYZ[\]^_
		0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // `abcdefg
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // hijklmno
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // pqrstuvw
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // xyz{|}~.
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // ........
};

// Mapping of nibble (half-byte) to ASCII
static const uint8_t nibble_to_ascii[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

// Tools to convert DLC enum (e.g. FDCAN_DLC_BYTES_0) in RX and TX header to uint8, and vice-versa
static const uint8_t DlcToUint8convTable[] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};

uint32_t  rawtoASCII(uint8_t* dst, const uint8_t* src, uint32_t raw_size)
{
	for (uint32_t i = 0; i < raw_size; i++)
	{
		dst[2*i] 	 = nibble_to_ascii[(src[i] >> 4  ) & 0xF];
		dst[(2*i)+1] = nibble_to_ascii[(src[i]   	 ) & 0xF];
	}
	return raw_size*2U;
}

uint32_t uint32toASCII(uint32_t src, uint8_t* dst)
{
	dst[0] = nibble_to_ascii[(src >> 28  ) & 0xF];
	dst[1] = nibble_to_ascii[(src >> 24  ) & 0xF];
	dst[2] = nibble_to_ascii[(src >> 20  ) & 0xF];
	dst[3] = nibble_to_ascii[(src >> 16  ) & 0xF];
	dst[4] = nibble_to_ascii[(src >> 12  ) & 0xF];
	dst[5] = nibble_to_ascii[(src >> 8   ) & 0xF];
	dst[6] = nibble_to_ascii[(src >> 4   ) & 0xF];
	dst[7] = nibble_to_ascii[(src        ) & 0xF];
	return 8U;
}

uint32_t uint12toASCII(uint16_t src, uint8_t* dst)
{
	dst[0] = nibble_to_ascii[(src >> 8 ) & 0xF];
	dst[1] = nibble_to_ascii[(src >> 4  ) & 0xF];
	dst[2] = nibble_to_ascii[(src       ) & 0xF];
	return 3U;
}

uint32_t uint16toASCII(uint16_t src, uint8_t* dst)
{
	dst[0] = nibble_to_ascii[(src >> 12 ) & 0xF];
	dst[1] = nibble_to_ascii[(src >> 8  ) & 0xF];
	dst[2] = nibble_to_ascii[(src >> 4  ) & 0xF];
	dst[3] = nibble_to_ascii[(src       ) & 0xF];
	return 4U;
}

uint32_t uint8toASCII(uint8_t src, uint8_t* dst)
{
	dst[0] = nibble_to_ascii[(src >> 4  ) & 0xF];
	dst[1] = nibble_to_ascii[(src       ) & 0xF];
	return 2U;
}

uint32_t uint4toASCII(uint8_t src, uint8_t* dst)
{
	dst[0] = nibble_to_ascii[(src       ) & 0xF];
	return 1U;
}

uint8_t uintToBCD(uint64_t val, char *dst)
{
    char *ptr = dst;
    char *start = dst;
    uint8_t result;

    if (val == 0) {
        *ptr++ = '0';
    } else {
        while (val > 0) {
            *ptr++ = (val % 10) + '0';
            val /= 10;
        }
    }
    result = ptr - dst;
    *ptr = '\0';

    // Reverse the string in place
    ptr--;
    while (start < ptr) {
        char temp = *start;
        *start++ = *ptr;
        *ptr-- = temp;
    }
    return result;
}

void ASCIItoRaw(uint8_t* dst, const uint8_t* src, uint32_t raw_size)
{
	for (uint32_t i = 0; i < raw_size; i++)
	{
		dst[i] = (ascii_hashmap[src[2*i]] << 4) + (ascii_hashmap[src[(2*i)+1]]);
	}
}

uint8_t ASCIItoUint4(const uint8_t* src)
{
	return (ascii_hashmap[src[0]]);
}

inline uint8_t ASCIItoUint8(const uint8_t* src)
{
	return (ascii_hashmap[src[0]] << 4) + (ascii_hashmap[src[1]]);
}

inline uint16_t ASCIItoUint16(const uint8_t* src)
{
	return (ascii_hashmap[src[0]] << 12)  + (ascii_hashmap[src[1]] << 8) + (ascii_hashmap[src[2]] << 4) + (ascii_hashmap[src[3]]);
}

inline uint16_t ASCIItoUint12(const uint8_t* src)
{
	return (ascii_hashmap[src[0]] << 8)  + (ascii_hashmap[src[1]] << 4) + (ascii_hashmap[src[2]]);
}

inline uint32_t ASCIItoUint32(const uint8_t* src)
{
	return (ascii_hashmap[src[0]] << 28)  + (ascii_hashmap[src[1]] << 24) + (ascii_hashmap[src[2]] << 20)  + (ascii_hashmap[src[3]] << 16)  + (ascii_hashmap[src[4]] << 12)  + (ascii_hashmap[src[5]] << 8) + (ascii_hashmap[src[6]] << 4) + (ascii_hashmap[src[7]]);
}

uint32_t convertBytesToUint32(uint8_t *data, uint16_t dataSize)
{
	if(dataSize < sizeof(uint32_t)) return 0;
	return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

uint32_t convertBytesToUint32_l(uint8_t *data, uint16_t dataSize)
{
	if(dataSize < sizeof(uint32_t)) return 0;
	return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
}

uint16_t convertBytesToUint16(uint8_t *data, uint16_t dataSize)
{
	if(dataSize < sizeof(uint16_t)) return 0;
	return (data[0] << 8) | data[1];
}

uint16_t convertUint16ToBytes(uint8_t *data, uint16_t dataSize, uint16_t n)
{
	if(dataSize < sizeof(uint16_t)) return 0;
	data[0] = (n & 0xFF00) >> 8;
	data[1] = n & 0xFF;
	return sizeof(uint16_t);
}

uint16_t convertUint32ToBytes(uint8_t *data, uint16_t dataSize, uint32_t n)
{
	if(dataSize < sizeof(uint32_t)) return 0;
	data[0] = n >> 24;
	data[1] = (n & 0x00FF0000) >> 16;
	data[2] = (n & 0x0000FF00) >> 8;
	data[3] = n & 0x000000FF;
	return sizeof(uint32_t);
}

uint8_t DLCtoUINT8(uint32_t dlc_enum)
{
	return DlcToUint8convTable[(uint8_t)(dlc_enum)];
}

uint32_t UINT8toDLC(uint8_t dlc)
{
	return dlc; // No conversion needed with new STM32 library
}

void RAMN_memset(void* dst, uint8_t byte, uint32_t size)
{
	uint8_t* p = (uint8_t *)dst;
	for(uint32_t i = 0; i < size; i++)
	{
		p[i] = byte;
	}
}

void RAMN_memcpy(void* dst, const void* src, uint32_t size)
{
	uint8_t* d = (uint8_t*)dst;
	const uint8_t* s = (const uint8_t*)src;
	for(uint32_t i = 0; i < size; i++) d[i] = s[i];
}

uint16_t RAMN_strlen(const char *str)
{
	uint32_t i;

	for(i = 0; ; i++) if(str[i] == '\0') break;
	return i;
}

uint16_t applyEndian16(uint16_t val)
{
#ifdef USE_BIG_ENDIAN_CAN
	return ((val&0xFF) << 8) | ((val >> 8)&0xFF);
#else
	return val;
#endif
}

void RAMN_TaskDelay(uint32_t msec)
{
	osDelay(pdMS_TO_TICKS(msec));
}

