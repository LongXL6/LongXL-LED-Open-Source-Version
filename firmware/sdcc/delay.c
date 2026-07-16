#include "types.h"

/* Microsecond-level delay (24 nops per loop, matching the original Keil build;
   only used to pace the key-scan loop, so precision is not critical) */
void delay_us(u16 us)
{
	while (us--)
	{
		__asm
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
		__endasm;
	}
}

void delay_ms(u16 ms)
{
	while (ms--)
	{
		delay_us(998);
	}
}
