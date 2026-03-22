/*
 * ramn_bitbang.c
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2026 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
 *
 * This software component is licensed by TOYOTA MOTOR CORPORATION under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include "ramn_bitbang.h"

#ifdef ENABLE_BITBANG

#include "ramn_canfd.h"
#include "ramn_usb.h"

// Timer to detect timeouts (~1.2 kHz)
#define TIMEOUT_TIM (TIM17)
// Timer for CAN bits (~80MHz)
#define CANBIT_TIM (TIM2)
const uint32_t PIN_RX = (1U << 8);
const uint32_t TX_REC = (1U << 9);
const uint32_t TX_DOM = (1U << (9 + 16));

// Special Arbitration IDs used to trigger an action (trigger on any ID, trigger immediately, trigger on bus idle)
#define ARBID_TRIGGER_ANY  0xFFFFFFFF
#define ARBID_TRIGGER_NOW  0xFFFFFFFE
#define ARBID_TRIGGER_IDLE 0xFFFFFFFD

// Number of bits to wait to consider the bus is idle (theoretically should be 11, but IFS0 can be skipped).
#define IDLE_RECESSIVE_BITS 10

// Buffer where CAN frame bits are stored (as '1' or '0')
#define BB_RX_BUFFER_SIZE  4000
char bb_can_bits[BB_RX_BUFFER_SIZE];
// Index to keep track of the current frame
uint32_t bb_can_index = 0;

// Bitbang module settings (default clock is 80MHz, below configuration for 500kbps)
static uint16_t prescaler = 1; 			 	// Actual prescaler (not register value, for which we subtract one).
static uint16_t bit_quanta = 160; 		 	// Total quanta per bit
static uint16_t sampling_quanta = 80; 	 	// Where to sample each CAN frame bit
static uint8_t 	extended = 0U; 				// Whether to trigger on extended or standard ID
static uint8_t 	RTR = 0U; 					// Whether to trigger on data frame or RTR frame
static uint32_t trig = ARBID_TRIGGER_ANY;  	// arbitration IDs to trigger on (see special arbitration IDs above)
static uint16_t timeout = 2000; 			// Timeout value (@~1.22 KHz)

// Function to generate the beginning of a bitstream for the provided parameters (arbitration ID, extended flag, RTR flag)
// Returns bitstream length.
uint16_t GenerateFirstFrameBits(uint32_t id, uint8_t ext, uint8_t rtr, char *out, uint16_t max_len)
{
	uint16_t idx = 0;
	uint8_t count = 0;
	char last = 0;

	// preprocessor macro to stuff bits automatically
#define ADD_BIT(b) \
		{ \
	char bit = (b); \
	if (idx >= max_len-1) return 0; \
	out[idx++] = bit; \
	if (bit == last) count++; \
	else { last = bit; count = 1; } \
	if (count == 5) \
	{ \
		char stuffed = (bit == '1') ? '0' : '1'; \
		if (idx >= max_len-1) return 0; \
		out[idx++] = stuffed; \
		last = stuffed; \
		count = 1; \
	} \
		}

	ADD_BIT('0') // SOF

	if (!ext)
	{
		for (int i = 10; i >= 0; i--) ADD_BIT(((id >> i) & 1) ? '1' : '0')

												ADD_BIT(rtr ? '1' : '0') // RTR
												ADD_BIT('0') // IDE
												ADD_BIT('0') // R0
	}
	else
	{
		for (int i = 28; i >= 18; i--) ADD_BIT(((id >> i) & 1) ? '1' : '0')

												ADD_BIT('1') // SRR
												ADD_BIT('1') // IDE

												for (int i = 17; i >= 0; i--) ADD_BIT(((id >> i) & 1) ? '1' : '0')

												ADD_BIT(rtr ? '1' : '0')
												ADD_BIT('0') // R1
												ADD_BIT('0') // R0
	}

	out[idx] = 0;
	return idx;

#undef ADD_BIT
}

// Sets up the bitbang module (timers and GPIOs)
static void BB_Start()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// Set up Timers
	CANBIT_TIM->CR1 &= ~TIM_CR1_CEN; // Disable timer
	CANBIT_TIM->PSC = (prescaler-1);  // Set new prescaler value
	CANBIT_TIM->CNT = 0; // Reset counter
	CANBIT_TIM->EGR = TIM_EGR_UG; // Generate update event to load prescaler
	CANBIT_TIM->CR1 |= TIM_CR1_CEN; // Enable timer

	TIMEOUT_TIM->CR1 &= ~TIM_CR1_CEN; // Disable timer
	TIMEOUT_TIM->PSC = 65535;  // Set new prescaler value //~1.22kHz
	TIMEOUT_TIM->CNT = 0; // Reset counter
	TIMEOUT_TIM->EGR = TIM_EGR_UG; // Generate update event to load prescaler
	TIMEOUT_TIM->CR1 |= TIM_CR1_CEN; // Enable timer

	// Set up PB8 (CAN RX) and PB9 (CAN TX) as GPIOs instead of CAN
	RAMN_FDCAN_Disable(); // Disable CAN peripheral
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET); 	// Make sure TX is high (recessive)

	GPIO_InitStruct.Pin  = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin   = GPIO_PIN_9;
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

// Stops the CAN module and reset the CAN peripheral
static void BB_Stop()
{
	RAMN_FDCAN_ResetPeripheral();
}

// This function hangs until CAN bus idle is detected.
// CAN Bus idle is defined as IDLE_RECESSIVE_BITS successive bits
// For better performances, we do not timeout.
// As a result, it might hang on wrong setups (bad baudrates).
__attribute__((optimize("Ofast"))) static inline void 			BB_WaitForIdleRecessive(void)
{
	uint16_t start;

	while (1)
	{
		while (!(GPIOB->IDR & (1 << 8))); // Wait until bus is recessive
		start = CANBIT_TIM->CNT;
		while (GPIOB->IDR & (1 << 8)) // Bus must stay recessive, otherwise reset timer
		{
			if ((CANBIT_TIM->CNT - start) >= (bit_quanta*IDLE_RECESSIVE_BITS)) return; // 7 bits at 500 kbit/s
		}
	}
}

// Function that returns True of the current CAN bitstream corresponds to the expected bitstream.
// Typically used to trigger on a specific ID, but can be extended to any bitstream.
__attribute__((optimize("Ofast"))) static inline RAMN_Bool_t 	BB_MatchBitstream(const char *expected_bits, uint32_t expected_len)
{
	uint32_t next_sample = sampling_quanta;
	bb_can_index = 0U;

	while (bb_can_index < expected_len)
	{
		while (CANBIT_TIM->CNT < next_sample);

		if (((GPIOB->IDR & (1U << 8)) ? '1' : '0') != expected_bits[bb_can_index]) return False;

		bb_can_bits[bb_can_index] = expected_bits[bb_can_index];
		bb_can_index++;
		next_sample += bit_quanta;
	}

	return True;
}

__attribute__((optimize("Ofast"))) static inline RAMN_Result_t 	BB_ReadUntilEOF(void)
{
	RAMN_Result_t result = RAMN_ERROR;
	uint8_t recessive_count = 0U;

	uint32_t next_sample = sampling_quanta + (bb_can_index * bit_quanta);

	while (bb_can_index < BB_RX_BUFFER_SIZE - 1)
	{
		while (CANBIT_TIM->CNT < next_sample);

		bb_can_bits[bb_can_index] = (GPIOB->IDR & (1U << 8)) ? '1' : '0';

		if (bb_can_bits[bb_can_index] == '1') recessive_count++;
		else recessive_count = 0U;

		bb_can_index++;
		next_sample += bit_quanta;

		if ((recessive_count > IDLE_RECESSIVE_BITS) && bb_can_index > 20U)
		{
			result = RAMN_OK;
			break;
		}
	}

	bb_can_bits[bb_can_index] = 0U;
	return result;
}

__attribute__((optimize("Ofast"))) static inline RAMN_Result_t Trigger(void)
{
	uint32_t stuffed_len;
	char first_bits[48];

	if (trig == ARBID_TRIGGER_NOW)
	{
		CANBIT_TIM->CNT = 0;
		return RAMN_OK;   // wait for SOF
	}
	else if (trig == ARBID_TRIGGER_IDLE)
	{
		BB_WaitForIdleRecessive();
		CANBIT_TIM->CNT = 0;
		return RAMN_OK;
	}
	else if (trig == ARBID_TRIGGER_ANY) stuffed_len = 0U; // Trigger on any Message
	else
	{
		stuffed_len = GenerateFirstFrameBits(trig, extended, RTR, first_bits, sizeof(first_bits));
		if (stuffed_len == 0) RAMN_USB_SendStringFromTask("Error generating expected bitstream\r");
	}

	do
	{
		BB_WaitForIdleRecessive();
		while (GPIOB->IDR & (1 << 8))
		{
			if (TIMEOUT_TIM->CNT > timeout) return RAMN_ERROR;   // Wait for SOF
		}
		CANBIT_TIM->CNT = 0;
	} while (BB_MatchBitstream(first_bits, stuffed_len) != True);

	return RAMN_OK;
}

void CAN_KeepOnlyBits(char *buffer)
{
	uint16_t r = 0;
	uint16_t w = 0;
	while (buffer[r] != 0)
	{
		char c = buffer[r++];

		if (c == '0' || c == '1') buffer[w++] = c;
		else if(c == '*') buffer[w++] = '1'; // restore trailing 1s
	}
	buffer[w] = 0;   // terminate string
}

RAMN_Bool_t CAN_Annotate(char *buffer)
{
	uint16_t end = 0;
	char last = 0;
	uint8_t count = 0;
	uint8_t stuffing = 1;
	uint16_t j;

	end = RAMN_strlen(buffer);
	if (end <= 20) return False;

	// Replace trailing 1s with '*' (bit-stuffing do not apply)
	j = end - 1;
	while (j >= 0 && buffer[j] == '1')
	{
		if ((end - j) > 12U) return False;
		buffer[j--] = '*';
	}

	// Replace stuffed bits with '_' and detect violations
	for (uint16_t i = 0; i < end; i++)
	{
		char b = buffer[i];
		if (b != '0' && b != '1')
			continue;

		if (!count) { last = b; count = 1; continue; }

		if (stuffing && count == 5)
		{
			if (b == last) return False;   	// Violation
			buffer[i] = '_';           		// Stuffed bit
			count = 0;
			continue;
		}

		count = (b == last) ? count + 1 : 1;
		last = b;

		if (stuffing && last == '1' && count >= 7)
			stuffing = 0;
	}

	// The code may have wrongly interpreted the ACK as a stuffing bit, fix if necessary
	for (int16_t p = end - 1; p >= 0; p--)
	{
		if (buffer[p] == '*') continue;
		if (buffer[p] == '_') buffer[p] = '0';
		break;
	}

	return True;
}

static uint32_t read_bits(const char *bits, uint32_t *idx, uint8_t n, char *buf)
{
	uint32_t v = 0;

	for (uint8_t i = 0; i < n; i++)
	{
		char b = bits[*idx];
		buf[i] = b;
		v = (v << 1) | (b == '1');
		(*idx)++;
	}

	buf[n] = 0;
	return v;
}

RAMN_Bool_t CAN_CheckCRC(const char *bits, uint32_t crc_start)
{
	uint16_t crc = 0;
	uint16_t crc_rx = 0;
	const uint16_t poly = 0x4599;

	for (uint32_t i = 0; i < crc_start; i++)
	{
		uint8_t bit = (bits[i] == '1');
		uint8_t msb = (crc >> 14) & 1;

		crc <<= 1;
		crc &= 0x7FFF;

		if (bit ^ msb)
			crc ^= poly;
	}

	for (uint8_t i = 0; i < 15; i++)
	{
		crc_rx <<= 1;
		crc_rx |= (bits[crc_start + i] == '1');
	}

	return (crc == crc_rx) ? True : False;
}

void CAN_PrintFrame(char *bits)
{
	uint32_t idx = 0;
	uint32_t id;
	uint32_t id_ext;
	uint8_t dlc;
	uint16_t crc;
	uint8_t rtr;

	char buf[32];
	char hex[9];

	uint32_t len = RAMN_strlen(bits);

	if (len < 19)
	{
		RAMN_USB_SendStringFromTask("Frame too short\r");
		return;
	}

	if (bits[0] != '0')
	{
		RAMN_USB_SendStringFromTask("Not a valid SOF\r");
		return;
	}

	RAMN_USB_SendStringFromTask("SOF         | 0\r");
	idx = 1;

	/* ID */
	id = read_bits(bits, &idx, 11, buf);

	RAMN_USB_SendStringFromTask("ID          | ");
	RAMN_USB_SendStringFromTask(buf);
	RAMN_USB_SendStringFromTask(" (0x");

	uint32toASCII(id,(uint8_t*)hex);
	hex[8] = 0;
	RAMN_USB_SendStringFromTask(hex);
	RAMN_USB_SendStringFromTask(")\r");

	char srr_rtr = bits[idx++];
	char ide = bits[idx++];

	buf[0] = srr_rtr; buf[1] = 0;

	RAMN_USB_SendStringFromTask("RTR/SRR     | ");
	RAMN_USB_SendStringFromTask(buf);
	RAMN_USB_SendStringFromTask("\r");

	buf[0] = ide;

	RAMN_USB_SendStringFromTask("IDE         | ");
	RAMN_USB_SendStringFromTask(buf);
	RAMN_USB_SendStringFromTask("\r");

	if (ide == '1')
	{
		/* Check extended header length */
		if (len < 39)
		{
			RAMN_USB_SendStringFromTask("Frame too short.\r");
			return;
		}

		id_ext = read_bits(bits, &idx, 18, buf);

		RAMN_USB_SendStringFromTask("ID_EXT      | ");
		RAMN_USB_SendStringFromTask(buf);
		RAMN_USB_SendStringFromTask(" (0x");

		uint32toASCII(id_ext,(uint8_t*)hex);
		hex[8] = 0;
		RAMN_USB_SendStringFromTask(hex);
		RAMN_USB_SendStringFromTask(")\r");

		id = (id << 18) | id_ext;

		rtr = bits[idx++];

		buf[0] = rtr; buf[1] = 0;

		RAMN_USB_SendStringFromTask("RTR         | ");
		RAMN_USB_SendStringFromTask(buf);
		RAMN_USB_SendStringFromTask("\r");

		buf[0] = bits[idx++];

		RAMN_USB_SendStringFromTask("r1          | ");
		RAMN_USB_SendStringFromTask(buf);
		RAMN_USB_SendStringFromTask("\r");

		buf[0] = bits[idx++];

		RAMN_USB_SendStringFromTask("r0          | ");
		RAMN_USB_SendStringFromTask(buf);
		RAMN_USB_SendStringFromTask("\r");
	}
	else
	{
		rtr = srr_rtr;

		buf[0] = bits[idx++]; buf[1] = 0;

		RAMN_USB_SendStringFromTask("r0          | ");
		RAMN_USB_SendStringFromTask(buf);
		RAMN_USB_SendStringFromTask("\r");
	}

	dlc = read_bits(bits, &idx, 4, buf);

	RAMN_USB_SendStringFromTask("DLC         | ");
	RAMN_USB_SendStringFromTask(buf);
	RAMN_USB_SendStringFromTask(" (0x");

	uint8toASCII(dlc,(uint8_t*)hex);
	hex[2] = 0;
	RAMN_USB_SendStringFromTask(hex);
	RAMN_USB_SendStringFromTask(")\r");

	uint32_t data_bits = dlc * 8;

	if (idx + data_bits + 15 > len)
	{
		RAMN_USB_SendStringFromTask("Frame too short for dlc.\r");
		return;
	}

	if (rtr == '0' && dlc <= 8)
	{
		for (uint8_t i = 0; i < dlc; i++)
		{
			uint8_t data = read_bits(bits, &idx, 8, buf);

			RAMN_USB_SendStringFromTask("DATA        | ");
			RAMN_USB_SendStringFromTask(buf);
			RAMN_USB_SendStringFromTask(" (0x");

			uint8toASCII(data,(uint8_t*)hex);
			hex[2] = 0;
			RAMN_USB_SendStringFromTask(hex);
			RAMN_USB_SendStringFromTask(")\r");
		}
	}
	else
	{
		idx += data_bits;
	}

	uint32_t crc_start = idx;
	crc = read_bits(bits, &idx, 15, buf);

	RAMN_USB_SendStringFromTask("CRC         | ");
	RAMN_USB_SendStringFromTask(buf);
	RAMN_USB_SendStringFromTask(" (0x");

	uint16toASCII(crc,(uint8_t*)hex);
	hex[4] = 0;
	RAMN_USB_SendStringFromTask(hex);
	RAMN_USB_SendStringFromTask(") ");

	if (CAN_CheckCRC(bits, crc_start)) RAMN_USB_SendStringFromTask("CRC OK\r");
	else RAMN_USB_SendStringFromTask("CRC BAD\r");

	/* Tail */
	uint8_t tail_len = 0;

	while ((bits[idx] == '0' || bits[idx] == '1') && tail_len < sizeof(buf)-1)
		buf[tail_len++] = bits[idx++];

	if (tail_len)
	{
		buf[tail_len] = 0;

		RAMN_USB_SendStringFromTask("TAIL        | ");
		RAMN_USB_SendStringFromTask(buf);
		RAMN_USB_SendStringFromTask("\r");
	}
}

RAMN_Result_t RAMN_BITBANG_Set(char* param)
{
	char *value;
	uint32_t v;
	RAMN_Bool_t ok;

	if (!param) return RAMN_ERROR;

	value = param + RAMN_strlen(param) + 1;
	if (*value == 0) return RAMN_ERROR;

	if (RAMN_streq(param, "trig") || RAMN_streq(param, "trigger"))
	{
		if (RAMN_streq(value, "now"))
		{
			trig = ARBID_TRIGGER_NOW;
			return RAMN_OK;
		}
		else if (RAMN_streq(value, "any"))
		{
			trig = ARBID_TRIGGER_ANY;
			return RAMN_OK;
		}
		else if (RAMN_streq(value, "idle"))
		{
			trig = ARBID_TRIGGER_IDLE;
			return RAMN_OK;
		}

		v = RAMN_strtoul(value, 16, &ok);
		if (!ok) return RAMN_ERROR;

		if (v > 0x1FFFFFFFUL) return RAMN_ERROR;

		if (v > 0x7FFUL && extended == 0U)
		{
			extended = 1U;
			RAMN_USB_SendStringFromTask("Warning: CAN ID > 7FF, enabling extended\r");
		}

		trig = v;
		return RAMN_OK;
	}

	if (RAMN_streq(param, "prescaler"))
	{
		v = RAMN_strtoul(value, 10, &ok);
		if (!ok || v == 0 || v > 0xFFFFUL) return RAMN_ERROR;
		prescaler = (uint16_t)v;
		return RAMN_OK;
	}

	if (RAMN_streq(param, "bit_quanta"))
	{
		v = RAMN_strtoul(value, 10, &ok);
		if (!ok || v == 0 || v > 0xFFFFUL) return RAMN_ERROR;
		bit_quanta = (uint16_t)v;
		if (sampling_quanta >= bit_quanta) sampling_quanta = bit_quanta >> 1;
		return RAMN_OK;
	}

	if (RAMN_streq(param, "sampling_quanta"))
	{
		v = RAMN_strtoul(value, 10, &ok);
		if (!ok || v >= bit_quanta) return RAMN_ERROR;

		sampling_quanta = (uint16_t)v;
		return RAMN_OK;
	}

	if (RAMN_streq(param, "extended"))
	{
		if (RAMN_streq(value, "0"))
		{
			if (trig > 0x7FFUL && trig <= 0x1FFFFFFFUL)
			{
				RAMN_USB_SendStringFromTask("Error: trigger requires extended ID\r");
				return RAMN_ERROR;
			}
			extended = 0U;
		}
		else if (RAMN_streq(value, "1")) extended = 1U;
		else return RAMN_ERROR;

		return RAMN_OK;
	}

	if (RAMN_streq(param, "rtr"))
	{
		if (RAMN_streq(value, "0")) RTR = 0U;
		else if (RAMN_streq(value, "1")) RTR = 1U;
		else return RAMN_ERROR;

		return RAMN_OK;
	}

	if (RAMN_streq(param, "timeout"))
	{
		v = RAMN_strtoul(value, 10, &ok);
		if (!ok || v > 0xFFFFUL) return RAMN_ERROR;
		timeout = (uint16_t)v;
		return RAMN_OK;
	}

	return RAMN_ERROR;
}

RAMN_Result_t RAMN_BITBANG_Show(void)
{
	char num[12];

	RAMN_USB_SendStringFromTask("\rtimeout ");
	uint32toBCD(timeout, num);
	RAMN_USB_SendStringFromTask(num);
	RAMN_USB_SendStringFromTask(" (ms~)\r");

	RAMN_USB_SendStringFromTask("trig ");
	uint32toASCII(trig, (uint8_t*)num);
	num[8] = 0;
	RAMN_USB_SendStringFromTask(num);

	if (trig == ARBID_TRIGGER_NOW) RAMN_USB_SendStringFromTask(" (now)");
	else if (trig == ARBID_TRIGGER_ANY) RAMN_USB_SendStringFromTask(" (any)");
	else if (trig == ARBID_TRIGGER_IDLE) RAMN_USB_SendStringFromTask(" (idle)");

	RAMN_USB_SendStringFromTask("\rextended ");
	uint32toBCD(extended, num);
	RAMN_USB_SendStringFromTask(num);

	RAMN_USB_SendStringFromTask("\rrtr ");
	uint32toBCD(RTR, num);
	RAMN_USB_SendStringFromTask(num);

	RAMN_USB_SendStringFromTask("\r\rCAN Timing settings:\rprescaler ");
	uint32toBCD(prescaler, num);
	RAMN_USB_SendStringFromTask(num);

	RAMN_USB_SendStringFromTask("\rbit_quanta ");
	uint32toBCD(bit_quanta, num);
	RAMN_USB_SendStringFromTask(num);

	RAMN_USB_SendStringFromTask("\rsampling_quanta ");
	uint32toBCD(sampling_quanta, num);
	RAMN_USB_SendStringFromTask(num);
	RAMN_USB_SendStringFromTask("\r");

	return RAMN_OK;
}

__attribute__((optimize("Ofast"))) RAMN_Result_t RAMN_BITBANG_Jam(void)
{
	BB_Start();
	TIMEOUT_TIM->CNT = 0;

	while (TIMEOUT_TIM->CNT < timeout)
	{
		CANBIT_TIM->CNT = 0;
		while (CANBIT_TIM->CNT < bit_quanta);
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
	}
	BB_Stop();
	return RAMN_OK;
}

__attribute__((optimize("Ofast"))) uint32_t RAMN_BITBANG_BusLoad(void)
{
	uint32_t start_time;
	uint32_t bus_occupied = 0;
	uint8_t  recessive_count;
	uint32_t next_sample;

	if (timeout <= 1000)
	{
		RAMN_USB_SendStringFromTask("Timeout value too short.\r");
		return 0U;
	}

	BB_Start();
	__disable_irq();

	if (Trigger() != RAMN_OK)
	{
		__enable_irq();
		RAMN_USB_SendStringFromTask("Trigger Timeout.\r");
		BB_Stop();
		return 0U;
	}

	TIMEOUT_TIM->CNT = 0;

	while (TIMEOUT_TIM->CNT < timeout)
	{
		// Wait for SOF
		while (GPIOB->IDR & (1U << 8))
		{
			if (TIMEOUT_TIM->CNT >= timeout)
			{
				__enable_irq();
				BB_Stop();
				return (bus_occupied * 100000U) / timeout;
			}
		}

		start_time = TIMEOUT_TIM->CNT; //We record timing of SOF bit transition
		CANBIT_TIM->CNT = 0;
		recessive_count = 0;
		next_sample = sampling_quanta;

		while (1)
		{
			while (CANBIT_TIM->CNT < next_sample);

			if (GPIOB->IDR & (1U << 8)) recessive_count++;
			else recessive_count = 0;

			next_sample += bit_quanta;

			if (recessive_count >= 11)
			{
				// Wait until next theoretically possible transition
				while (CANBIT_TIM->CNT < (next_sample-sampling_quanta));
				break;
			}
		}
		bus_occupied += (uint32_t)(TIMEOUT_TIM->CNT - start_time);
	}

	__enable_irq();
	BB_Stop();
	return (bus_occupied * 100000U) / timeout;
}

inline RAMN_Result_t RAMN_BITBANG_Read(void)
{
	if ((trig == ARBID_TRIGGER_NOW) || (trig == ARBID_TRIGGER_IDLE))
	{
		RAMN_USB_SendStringFromTask("Error: trigger not compatible with this module.\r");
		return RAMN_ERROR;
	}

	BB_Start();
	__disable_irq();

	TIMEOUT_TIM->CNT = 0;

	if (Trigger() != RAMN_OK)
	{
		__enable_irq();
		RAMN_USB_SendStringFromTask("Error: Trigger Timeout.\r");
		BB_Stop();
		return RAMN_ERROR;
	}

	if (BB_ReadUntilEOF() != RAMN_OK)
	{
		__enable_irq();
		RAMN_USB_SendStringFromTask("Could not detect EOF\r");
		BB_Stop();
		return RAMN_ERROR;
	}

	__enable_irq();

	if (bb_can_index <= 7)
	{
		RAMN_USB_SendStringFromTask("Bit string too short\r");
		BB_Stop();
		return RAMN_OK;
	}

	RAMN_USB_SendStringFromTask(bb_can_bits);
	RAMN_USB_SendStringFromTask(" (Raw)\r");

	bb_can_bits[bb_can_index] = 0;

	if (!CAN_Annotate(bb_can_bits))
	{
		RAMN_USB_SendStringFromTask("BAD STUFFING\r");
		BB_Stop();
		return RAMN_OK;
	}

	RAMN_USB_SendStringFromTask(bb_can_bits);
	RAMN_USB_SendStringFromTask(" (Destuffed)\r\r");

	CAN_KeepOnlyBits(bb_can_bits);
	CAN_PrintFrame(bb_can_bits);

	BB_Stop();
	return RAMN_OK;
}

__attribute__((optimize("Ofast"))) inline RAMN_Result_t RAMN_BITBANG_Dump(void)
{
	uint32_t next_sample;

	BB_Start();
	__disable_irq();

	CANBIT_TIM->CNT = 0;
	bb_can_index = 0;

	TIMEOUT_TIM->CNT = 0;
	if (Trigger() != RAMN_OK)
	{
		__enable_irq();
		RAMN_USB_SendStringFromTask("Trigger Timeout.\r");
		BB_Stop();
		return RAMN_ERROR;
	}

	next_sample = sampling_quanta + (bb_can_index * bit_quanta);

	while (bb_can_index < BB_RX_BUFFER_SIZE - 1)
	{
		while (CANBIT_TIM->CNT < next_sample);

		bb_can_bits[bb_can_index] = (GPIOB->IDR & (1U << 8)) ? '1' : '0';
		bb_can_index++;
		next_sample += bit_quanta;
	}

	bb_can_bits[bb_can_index] = 0;

	__enable_irq();
	BB_Stop();

	RAMN_USB_SendStringFromTask(bb_can_bits);
	RAMN_USB_SendStringFromTask("\r");

	return RAMN_OK;
}

__attribute__((optimize("Ofast"))) inline RAMN_Result_t RAMN_BITBANG_Send(char *param)
{
	uint32_t tx_len;
	uint32_t next_tx;
	uint32_t next_rx;

	if (!param) return RAMN_ERROR;

	tx_len = RAMN_strlen(param);
	if (tx_len > BB_RX_BUFFER_SIZE) tx_len = BB_RX_BUFFER_SIZE;

	if (trig != ARBID_TRIGGER_IDLE) RAMN_USB_SendStringFromTask("Warning: you should use the 'idle' trigger.\r");

	BB_Start();
	__disable_irq();

	CANBIT_TIM->CNT = 0;
	TIMEOUT_TIM->CNT = 0;
	bb_can_index = 0;

	if (Trigger() != RAMN_OK)
	{
		__enable_irq();
		RAMN_USB_SendStringFromTask("Error: Trigger Timeout.\r");
		BB_Stop();
		return RAMN_ERROR;
	}

	bb_can_index = 0;

	next_tx = bit_quanta;
	next_rx = bit_quanta + sampling_quanta;

	while (bb_can_index < BB_RX_BUFFER_SIZE - 1)
	{
		while (CANBIT_TIM->CNT < next_tx);

		if (bb_can_index < tx_len)
		{
			if (param[bb_can_index] == '1') GPIOB->BSRR = TX_REC;
			else if (param[bb_can_index] == '0') GPIOB->BSRR = TX_DOM;
		}
		else GPIOB->BSRR = TX_REC;

		while (CANBIT_TIM->CNT < next_rx);

		bb_can_bits[bb_can_index] = (GPIOB->IDR & (1U << 8)) ? '1' : '0';

		bb_can_index++;
		next_tx += bit_quanta;
		next_rx += bit_quanta;
	}

	bb_can_bits[bb_can_index] = 0;

	// Put back to Recessive
	while (CANBIT_TIM->CNT < next_tx);
	GPIOB->BSRR = TX_REC;

	__enable_irq();
	BB_Stop();

	RAMN_USB_SendStringFromTask(bb_can_bits);
	RAMN_USB_SendStringFromTask("\r");

	return RAMN_OK;
}

#define LOOP_REPETITION_SIZE 16U
__attribute__((optimize("Ofast"))) inline RAMN_Result_t RAMN_BITBANG_LoopOF(void)
{
	uint32_t next_tx;
	uint32_t next_rx;
	uint32_t recessive_seen = 0;

	BB_Start();
	__disable_irq();

	TIMEOUT_TIM->CNT = 0;

	if (Trigger() != RAMN_OK)
	{
		__enable_irq();
		RAMN_USB_SendStringFromTask("Error: Trigger Timeout.\r");
		BB_Stop();
		return RAMN_ERROR;
	}

	// Wait for 9 recessives
	next_rx = (bb_can_index * bit_quanta) + sampling_quanta;
	while (recessive_seen < 9)
	{
		while (CANBIT_TIM->CNT < next_rx);

		uint8_t bit = (GPIOB->IDR & PIN_RX) ? 1U : 0U;

		if (bit) recessive_seen++;
		else recessive_seen = 0;

		if (bb_can_index < BB_RX_BUFFER_SIZE - 1) bb_can_bits[bb_can_index] = bit ? '1' : '0';

		bb_can_index++;
		next_rx += bit_quanta;
	}

	next_tx = next_rx - sampling_quanta;

	// We enter a loop where we repeat 6 dominants and 10 recessives
	// Note that we actually observe 7 dominants (since other ECUs also send 6 dominants after our first).
	while (TIMEOUT_TIM->CNT <= timeout && bb_can_index < (BB_RX_BUFFER_SIZE - (LOOP_REPETITION_SIZE+1)))
	{
		for (uint32_t i = 0; i < 16; i++)
		{
			while (CANBIT_TIM->CNT < next_tx);

			if (i < 6) GPIOB->BSRR = TX_DOM; // technically, 1 should be  enough, but we send 6.
			else GPIOB->BSRR = TX_REC;

			while (CANBIT_TIM->CNT < next_rx);

			uint8_t bit = (GPIOB->IDR & PIN_RX) ? 1U : 0U;
			bb_can_bits[bb_can_index] = bit ? '1' : '0';

			bb_can_index++;

			next_tx += bit_quanta;
			next_rx += bit_quanta;

			if (TIMEOUT_TIM->CNT > timeout) break;
		}
	}

	// If we wan out of place to save rx data, just continue without saving
	while (CANBIT_TIM->CNT < next_tx);
	while (TIMEOUT_TIM->CNT <= timeout)
	{
		CANBIT_TIM->CNT = 0;
		GPIOB->BSRR = TX_DOM; while (CANBIT_TIM->CNT < (6U * bit_quanta));
		GPIOB->BSRR = TX_REC; while (CANBIT_TIM->CNT < (LOOP_REPETITION_SIZE * bit_quanta));
	}

	if (bb_can_index < BB_RX_BUFFER_SIZE) bb_can_bits[bb_can_index] = 0;
	else bb_can_bits[BB_RX_BUFFER_SIZE - 1] = 0;

	__enable_irq();
	BB_Stop();

	RAMN_USB_SendStringFromTask(bb_can_bits);
	RAMN_USB_SendStringFromTask("\r");

	return RAMN_OK;
}

__attribute__((optimize("Ofast"))) inline RAMN_Result_t RAMN_BITBANG_DenyOnce(uint8_t n)
{
	uint32_t next_tx;
	uint32_t next_rx;
	uint32_t recessive_seen;
	uint32_t target = 6U + n;
	uint8_t bit;

	if ((trig == ARBID_TRIGGER_IDLE) || (trig == ARBID_TRIGGER_NOW))
	{
		RAMN_USB_SendStringFromTask("Invalid trigger.\r");
		return RAMN_ERROR;
	}

	BB_Start();
	__disable_irq();

	TIMEOUT_TIM->CNT = 0;

	bb_can_index = 0;
	recessive_seen = 0;
	CANBIT_TIM->CNT = 0;

	if (Trigger() == RAMN_OK)
	{
		next_rx = (bit_quanta * bb_can_index) + sampling_quanta;

		while (recessive_seen < target)
		{
			while (CANBIT_TIM->CNT < next_rx);

			bit = (GPIOB->IDR & PIN_RX) ? 1U : 0U;

			if (bit) recessive_seen++;
			else recessive_seen = 0;

			if (bb_can_index < BB_RX_BUFFER_SIZE - 1)
				bb_can_bits[bb_can_index] = bit ? '1' : '0';

			bb_can_index++;
			next_rx += bit_quanta;
		}

		next_tx = next_rx - sampling_quanta;

		while (CANBIT_TIM->CNT < next_tx);
		GPIOB->BSRR = TX_DOM;

		while (CANBIT_TIM->CNT < next_rx);

		bit = (GPIOB->IDR & PIN_RX) ? 1U : 0U;

		if (bb_can_index < BB_RX_BUFFER_SIZE - 1)
			bb_can_bits[bb_can_index] = bit ? '1' : '0';

		bb_can_index++;
		next_tx += bit_quanta;
		next_rx += bit_quanta;

		while (bb_can_index < BB_RX_BUFFER_SIZE - 1)
		{
			while (CANBIT_TIM->CNT < next_tx);
			GPIOB->BSRR = TX_REC;

			while (CANBIT_TIM->CNT < next_rx);

			bit = (GPIOB->IDR & PIN_RX) ? 1U : 0U;

			bb_can_bits[bb_can_index] = bit ? '1' : '0';

			bb_can_index++;
			next_tx += bit_quanta;
			next_rx += bit_quanta;
		}
	} else RAMN_USB_SendStringFromTask("Trigger timeout.\r");

	if (bb_can_index < BB_RX_BUFFER_SIZE) bb_can_bits[bb_can_index] = 0;
	else bb_can_bits[BB_RX_BUFFER_SIZE - 1] = 0;

	__enable_irq();
	BB_Stop();

	RAMN_USB_SendStringFromTask(bb_can_bits);
	RAMN_USB_SendStringFromTask("\r");

	return RAMN_OK;
}

__attribute__((optimize("Ofast"))) inline RAMN_Result_t RAMN_BITBANG_Deny(uint8_t n)
{
	uint32_t next_tx;
	uint32_t next_rx;
	uint32_t recessive_seen;
	uint32_t target = 6U + n;

	if ((trig == ARBID_TRIGGER_IDLE) || (trig == ARBID_TRIGGER_NOW))
	{
		RAMN_USB_SendStringFromTask("Invalid trigger.\r");
		return RAMN_ERROR;
	}

	BB_Start();
	__disable_irq();

	TIMEOUT_TIM->CNT = 0;

	while (TIMEOUT_TIM->CNT < timeout)
	{
		recessive_seen = 0;
		CANBIT_TIM->CNT = 0;
		bb_can_index = 0;

		if (Trigger() != RAMN_OK)
			break;

		next_rx = (bit_quanta * bb_can_index) + sampling_quanta;

		while (recessive_seen < target)
		{
			while (CANBIT_TIM->CNT < next_rx);

			if ((GPIOB->IDR & PIN_RX)) recessive_seen++;
			else recessive_seen = 0;
			bb_can_index++;
			next_rx += bit_quanta;
		}

		// Set dominant for one bit
		next_tx = next_rx - sampling_quanta;
		while (CANBIT_TIM->CNT < next_tx);
		GPIOB->BSRR = TX_DOM;
		next_tx += bit_quanta;
		while (CANBIT_TIM->CNT < next_tx);
		GPIOB->BSRR = TX_REC;
	}

	__enable_irq();
	BB_Stop();

	return RAMN_OK;
}

#endif
