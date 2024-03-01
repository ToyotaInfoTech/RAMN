/*
 * ramn_utils.c
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 TOYOTA MOTOR CORPORATION.
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
#include "string.h"


// mapping of ASCII characters to hex values
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

//mapping of nibble (half-byte) to ASCII
static const uint8_t nibble_to_ascii[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

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

//tools to convert DLC enum (e.g. FDCAN_DLC_BYTES_0) in RX and TX header to uint8, and vice-versa
const uint8_t DlcToUint8convTable[] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};
uint8_t DLCtoUINT8(uint32_t dlc_enum)
{
	return DlcToUint8convTable[(uint8_t)(dlc_enum)];
}

uint32_t UINT8toDLC(uint8_t dlc)
{
	return dlc; //no conversion needed with new STM32 library
}

void RAMN_memcpy(uint8_t* dst, const uint8_t* src, uint32_t size)
{
	for(uint32_t i = 0; i < size; i++) dst[i] = src[i];
}

