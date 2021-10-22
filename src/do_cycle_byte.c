#include <stdint.h>

#define MSP430_SR_C (1 << 0)
#define MSP430_SR_Z (1 << 1)
#define MSP430_SR_N (1 << 2)
#define MSP430_SR_V (1 << 8)

// r0 = pc
// r1 = sp
// r2 = sr
typedef struct {
	uint16_t reg[16];
	uint8_t *mem;
} msp430_t;

extern uint16_t *msp430_do_cycle_get_single_operand(msp430_t *state, uint16_t opcode);
extern uint16_t *msp430_do_cycle_get_source_operand(msp430_t *state, uint16_t opcode);
extern uint16_t *msp430_do_cycle_get_dest_operand(msp430_t *state, uint16_t opcode);

int msp430_do_cycle_single_operand_byte(msp430_t *state, uint16_t opcode)
{
	uint8_t *operand = (uint8_t *)msp430_do_cycle_get_single_operand(state, opcode);
	if (operand == 0)
		return -1;

	switch ((opcode & 0x0380) >> 7) {
	case 0: {
		// RRC
		uint8_t res = *operand;
		uint16_t sr = 0;
		if (res & 1)
			sr |= MSP430_SR_C;
		res >>= 1;
		if (state->reg[2] & MSP430_SR_C)
			res |= 0x80;
		if (res == 0)
			sr |= MSP430_SR_Z;
		if ((int8_t)res < 0)
			sr |= MSP430_SR_N;
		*operand = res;
		state->reg[2] = sr;
		break; }
	case 2: {
		// RRA
		uint8_t res = *operand;
		uint16_t sr = 0;
		if (res & 1)
			sr |= MSP430_SR_C;
		res >>= 1;
		if (res == 0)
			sr |= MSP430_SR_Z;
		if ((int8_t)res < 0)
			sr |= MSP430_SR_N;
		*operand = res;
		state->reg[2] = sr;
		break; }
	case 4: {
		// PUSH
		uint16_t sp = state->reg[1] - 2;
		*((uint16_t *)(state->mem + sp)) = *operand;
		state->reg[1] = sp;
		break; }
	default:
		return -1;
		break;
	}

	return 0;
}

int msp430_do_cycle_dual_operand_byte(msp430_t *state, uint16_t opcode)
{
	uint8_t *src = (uint8_t *)msp430_do_cycle_get_source_operand(state, opcode);
	uint8_t *dst = (uint8_t *)msp430_do_cycle_get_dest_operand(state, opcode);
	if (src == 0 || dst == 0)
		return -1;

	switch ((opcode & 0xF000) >> 12) {
	case 4:
		// MOV
		*dst = *src;
		break;
	case 5: {
		// ADD
		uint16_t res = *src + *dst;
		uint16_t sr = 0;
		if ((int8_t)res < 0)
			sr |= MSP430_SR_N;
		if (res == 0)
			sr |= MSP430_SR_Z;
		if ((res & 0xFF00) != 0)
			sr |= MSP430_SR_C;
		if (((*src & 0x80) ^ (*src & 0x80)) == 0 && (*src & 0x80) != (res & 0x80))
			sr |= MSP430_SR_V;
		*dst = res & 0xFF;
		state->reg[2] =	sr;
		break; }
	case 6: {
		// ADDC
		uint16_t res = *src + *dst + (state->reg[2] & MSP430_SR_C);
		uint16_t sr = 0;
		if ((int8_t)res < 0)
			sr |= MSP430_SR_N;
		if (res == 0)
			sr |= MSP430_SR_Z;
		if ((res & 0xFF00) != 0)
			sr |= MSP430_SR_C;
		if (((*src & 0x80) ^ (*src & 0x80)) == 0 && (*src & 0x80) != (res & 0x80))
			sr |= MSP430_SR_V;
		*dst = res & 0xFF;
		state->reg[2] =	sr;
		break; }
	case 7: {
		// SUBC
		uint16_t res = *dst + ~(*src) + (state->reg[2] & MSP430_SR_C);
		uint16_t sr = 0;
		if ((int8_t)res < 0)
			sr |= MSP430_SR_N;
		if (res == 0)
			sr |= MSP430_SR_Z;
		if ((res & 0xFF00) != 0)
			sr |= MSP430_SR_C;
		if (((*src & 0x80) ^ (*src & 0x80)) == 0 && (*src & 0x80) != (res & 0x80))
			sr |= MSP430_SR_V;
		*dst = res & 0xFF;
		state->reg[2] =	sr;
		break; }
	case 8: {
		// SUB
		uint16_t res = *dst + ~(*src) + 1;
		uint16_t sr = 0;
		if ((int8_t)res < 0)
			sr |= MSP430_SR_N;
		if (res == 0)
			sr |= MSP430_SR_Z;
		if ((res & 0xFF00) != 0) // TODO confirm
			sr |= MSP430_SR_C;
		if (((*src & 0x80) ^ (*src & 0x80)) == 0 && (*src & 0x80) != (res & 0x80))
			sr |= MSP430_SR_V;
		*dst = res & 0xFF;
		state->reg[2] =	sr;
		break; }
	case 9: {
		// CMP
		uint16_t res = *dst + ~(*src) + 1;
		uint16_t sr = 0;
		if ((int8_t)res < 0)
			sr |= MSP430_SR_N;
		if (res == 0)
			sr |= MSP430_SR_Z;
		if ((res & 0xFF00) != 0)
			sr |= MSP430_SR_C;
		if (((*src & 0x80) ^ (*src & 0x80)) == 0 && (*src & 0x80) != (res & 0x80))
			sr |= MSP430_SR_V;
		state->reg[2] =	sr;
		break; }
	case 10:
		// DADD TODO
		break;
	case 11: {
		// BIT
		uint8_t res = *dst & *src;
		uint16_t sr = 0;
		if (res & 0x80)
			sr |= MSP430_SR_N;
		if (res == 0)
			sr |= MSP430_SR_Z;
		if (res != 0)
			sr |= MSP430_SR_C;
		state->reg[2] =	sr;
		break; }
	case 12:
		// BIC
		*dst &= ~(*src);
		break;
	case 13:
		// BIS
		*dst |= *src;
		break;
	case 14: {
		// XOR
		uint8_t res = *dst ^ *src;
		uint16_t sr = 0;
		if (res & 0x80)
			sr |= MSP430_SR_N;
		if (res == 0)
			sr |= MSP430_SR_Z;
		if (res != 0)
			sr |= MSP430_SR_C;
		if ((*dst & 0x80) && (*src & 0x80))
			sr |= MSP430_SR_V;
		*dst = res;
		state->reg[2] =	sr;
		break; }
	case 15: {
		// AND
		uint8_t res = *dst & *src;
		uint16_t sr = 0;
		if (res & 0x80)
			sr |= MSP430_SR_N;
		if (res == 0)
			sr |= MSP430_SR_Z;
		if (res != 0)
			sr |= MSP430_SR_C;
		*dst = res;
		state->reg[2] =	sr;
		break; }
	default:
		return -1;
	}

	return 0;
}

