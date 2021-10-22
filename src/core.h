#ifndef CORE_H
#define CORE_H

#include <stdint.h>

typedef struct {
	uint16_t reg[16];
	uint8_t *mem;
} msp430_t;

void msp430_init_state(msp430_t *state, void *mem);
int msp430_do_cycle(msp430_t *state);

#endif // CORE_H

