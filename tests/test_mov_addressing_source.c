#include "core.h"
#include "test.h"

#include <stdio.h>

static uint8_t mem[0x10000] = {
	TESTBIN
};

static void dump_state(msp430_t *state)
{
	puts("MSP430 dump state:");
	printf("R0/PC: 0x%04x R1/SP: 0x%04x R2/SR: 0x%04x R3:  0x%04x\n",
		state->reg[0], state->reg[1], state->reg[2], state->reg[3]);
	printf("R4:    0x%04x R5:    0x%04x R6:    0x%04x R7:  0x%04x\n",
		state->reg[4], state->reg[5], state->reg[6], state->reg[7]);
	printf("R8:    0x%04x R9:    0x%04x R10:   0x%04x R11: 0x%04x\n",
		state->reg[8], state->reg[9], state->reg[10], state->reg[11]);
	printf("R12:   0x%04x R13:   0x%04x R14:   0x%04x R15: 0x%04x\n\n",
		state->reg[12], state->reg[13], state->reg[14], state->reg[15]);
}

int main()
{
	msp430_t state;
	msp430_init_state(&state, mem);

	int r;
	do {
		r = msp430_do_cycle(&state);
		if (r < 0) {
			printf("Failed to execute near PC=0x%04x!\n", state.reg[0]);
			return 1;
		}
	} while (r != 1);

	if (state.reg[4] != 0x40 ||
	    state.reg[5] != 0x42 ||
	    state.reg[6] != 46 ||
	    state.reg[7] != 42 ||
	    state.reg[8] != 44 ||
	    state.reg[9] != 13 ||
	    state.reg[10] != 46)
	{
		dump_state(&state);
		return 1;
	}


	return 0;
}

