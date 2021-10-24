#include "core.h"
#include "test.h"

#include <stdio.h>

static uint8_t mem[0x10000] = {
	TESTBIN
};

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

	if (state.reg[4] != 0 ||
	    state.reg[5] != 1 ||
	    state.reg[6] != 2 ||
	    state.reg[7] != 0xFFFF ||
	    state.reg[8] != 4 ||
	    state.reg[9] != 8)
	{
		return 1;
	}

	return 0;
}

