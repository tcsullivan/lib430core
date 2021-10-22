#include "core.h"

#include <stdint.h>

#define MSP430_SR_C (1 << 0)
#define MSP430_SR_Z (1 << 1)
#define MSP430_SR_N (1 << 2)
#define MSP430_SR_V (1 << 8)

// r0 = pc
// r1 = sp
// r2 = sr

static int msp430_do_cycle_jump(msp430_t *state, uint16_t opcode);
int msp430_do_cycle_single_operand(msp430_t *state, uint16_t opcode);
int msp430_do_cycle_dual_operand(msp430_t *state, uint16_t opcode);
int msp430_do_cycle_single_operand_byte(msp430_t *state, uint16_t opcode);
int msp430_do_cycle_dual_operand_byte(msp430_t *state, uint16_t opcode);

uint16_t *msp430_do_cycle_get_single_operand(msp430_t *state, uint16_t opcode);
uint16_t *msp430_do_cycle_get_source_operand(msp430_t *state, uint16_t opcode);
uint16_t *msp430_do_cycle_get_dest_operand(msp430_t *state, uint16_t opcode);

static inline int msp430_get_opcode_bw(uint16_t opcode);

void msp430_init_state(msp430_t *state, void *mem)
{
	for (int i = 0; i < 16; ++i)
		state->reg[i] = 0;
	state->mem = mem;
}

int msp430_do_cycle(msp430_t *state)
{
	uint16_t pc = state->reg[0];
	uint16_t opcode = *((uint16_t *)(state->mem + pc));

	// Check for program end...
	if (opcode == 0x4130 && state->reg[1] == 0)
		return 1;

	state->reg[0] = pc + 2;

	int ret;
	if ((opcode & 0xE000) == 0x2000) {
		ret = msp430_do_cycle_jump(state, opcode);
	} else {
		int bw = msp430_get_opcode_bw(opcode);
		if (bw == 0) {
			if ((opcode & 0xFC00) == 0x1000)
				ret = msp430_do_cycle_single_operand(state, opcode);
			else
				ret = msp430_do_cycle_dual_operand(state, opcode);
		} else {
			if ((opcode & 0xFC00) == 0x1000)
				ret = msp430_do_cycle_single_operand_byte(state, opcode);
			else
				ret = msp430_do_cycle_dual_operand_byte(state, opcode);
		}
	}

	return ret;
}

static void msp430_jump_to_offset(msp430_t *state, uint16_t opcode)
{
	uint16_t addr = opcode & 0x3FF;
	if (addr & (1 << 9))
		addr |= 0xFC00;
	int16_t saddr = (int16_t)addr * 2;
	state->reg[0] += saddr;
}

int msp430_do_cycle_jump(msp430_t *state, uint16_t opcode)
{
	// PC to become PC + ((opcode & 0x3FF) << 1)
	switch ((opcode & 0x01C0) >> 10) {
	case 0:
		// JNE/JZ
		if ((state->reg[2] & MSP430_SR_Z) == 0)
			msp430_jump_to_offset(state, opcode);
		break;
	case 1:
		// JEQ/JZ
		if ((state->reg[2] & MSP430_SR_Z) == MSP430_SR_Z)
			msp430_jump_to_offset(state, opcode);
		break;
	case 2:
		// JNC/JLO
		if ((state->reg[2] & MSP430_SR_C) == 0)
			msp430_jump_to_offset(state, opcode);
		break;
	case 3:
		// JC/JHS
		if ((state->reg[2] & MSP430_SR_C) == MSP430_SR_C)
			msp430_jump_to_offset(state, opcode);
		break;
	case 4:
		// JN
		if ((state->reg[2] & MSP430_SR_N) == MSP430_SR_N)
			msp430_jump_to_offset(state, opcode);
		break;
	case 5: {
		// JGE
		uint16_t flags = state->reg[2] & (MSP430_SR_N | MSP430_SR_V);
		if (flags == 0 || flags == (MSP430_SR_N | MSP430_SR_V))
			msp430_jump_to_offset(state, opcode);
		break; }
	case 6: {
		// JL
		uint16_t flags = state->reg[2] & (MSP430_SR_N | MSP430_SR_V);
		if (flags == MSP430_SR_N || flags == MSP430_SR_V)
			msp430_jump_to_offset(state, opcode);
		break; }
	case 7:
		// JMP
		msp430_jump_to_offset(state, opcode);
		break;
	default:
		return -1;
	}

	return 0;
}

uint16_t *msp430_do_cycle_get_operand(msp430_t *state, int AS, int BW, int SOURCE)
{
	static uint16_t constants[6] = {
		0, 1, 2, -1, 4, 8
	};

	if (SOURCE == 0) {
		if (AS == 1) {
			// Operand at PC + X
			// X = word after PC
			uint16_t offset = *((uint16_t *)(state->mem + state->reg[0]));
			uint16_t *ret = (uint16_t *)(state->mem + state->reg[0] + offset);
			state->reg[0] += 2;
			return ret;
		} else if (AS == 3) {
			// Operand is X
			uint16_t *ret = (uint16_t *)(state->mem + state->reg[0]);
			state->reg[0] += 2;
			return ret;
		}
	}

	if (SOURCE == 2) {
		switch (AS) {
		case 0:
			// R2
			return &state->reg[2];
			break;
		case 1: {
			// Operand at X
			uint16_t offset = *((uint16_t *)(state->mem + state->reg[0]));
			uint16_t *ret = (uint16_t *)(state->mem + offset);
			state->reg[0] += 2;
			return ret;
			break; }
		case 2:
			return &constants[4];
			break;
		case 3:
			return &constants[5];
			break;
		}
	} else if (SOURCE == 3) {
		// Constants
		return &constants[AS];
	} else {
		switch (AS) {
		case 0:
			// Register
			return &state->reg[SOURCE];
			break;
		case 1: {
			// Operand at Rn + X
			uint16_t offset = *((uint16_t *)(state->mem + state->reg[0]));
			uint16_t *ret = (uint16_t *)(state->mem + state->reg[SOURCE] + offset);
			state->reg[0] += 2;
			return ret;
			break; }
		case 2:
			// Operand at Rn
			return (uint16_t *)(state->mem + state->reg[SOURCE]);
			break;
		case 3: {
			// Operand at Rn, increment Rn
			uint16_t *ret = (uint16_t *)(state->mem + state->reg[SOURCE]);
			state->reg[SOURCE] += BW ? 1 : 2;
			return ret;
			break; }
		}
	}

	return 0; // Failed...
}

inline int msp430_get_opcode_bw(uint16_t opcode)
{
	return opcode & (1 << 6);
}

uint16_t *msp430_do_cycle_get_single_operand(msp430_t *state, uint16_t opcode)
{
	return msp430_do_cycle_get_operand(state,
		/* AS */     (opcode & 0x30) >> 4,
		/* BW */     msp430_get_opcode_bw(opcode),
		/* SOURCE */ (opcode & 0xF));
}

uint16_t *msp430_do_cycle_get_source_operand(msp430_t *state, uint16_t opcode)
{
	return msp430_do_cycle_get_operand(state,
		/* AS */     (opcode & 0x30) >> 4,
		/* BW */     msp430_get_opcode_bw(opcode),
		/* SOURCE */ (opcode & 0xF00) >> 8);
}
uint16_t *msp430_do_cycle_get_dest_operand(msp430_t *state, uint16_t opcode)
{
	int AD = opcode & 0x80;
	int DEST = opcode & 0xF;

	if (AD == 0) {
		// Byte operation? Clear MSB.
		if (msp430_get_opcode_bw(opcode) == 0) {
			return &state->reg[DEST];
		} else {
			uint16_t *ret = &state->reg[DEST];
			*ret &= 0xFF;
			return ret;
		}
	} else {
		if (DEST == 0) {
			// dest at PC + X
			uint16_t offset = *((uint16_t *)(state->mem + state->reg[0]));
			uint16_t *ret = (uint16_t *)(state->mem + state->reg[0] + offset);
			state->reg[0] += 2;
			return ret;
		} else if (DEST == 2) {
			// dest at X
			uint16_t offset = *((uint16_t *)(state->mem + state->reg[0]));
			uint16_t *ret = (uint16_t *)(state->mem + offset);
			state->reg[0] += 2;
			return ret;
		} else {
			// dest at Rn + X
			uint16_t offset = *((uint16_t *)(state->mem + state->reg[0]));
			uint16_t *ret = (uint16_t *)(state->mem + state->reg[DEST] + offset);
			state->reg[0] += 2;
			return ret;
		}
	}

	return 0;
}

