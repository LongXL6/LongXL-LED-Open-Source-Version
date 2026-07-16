#include "config.h"

/* Microsecond-level delay. us = delay time.
   24 NOPs per loop, tuned for the 35 MHz internal RC oscillator.
   Only used to pace the key-scan loop, so precision is not critical. */
void delay_us(u16 us)
{
	while (us--)
	{
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
	}
}

/* Millisecond-level delay. ms = delay time. */
void delay_ms(u16 ms)
{
    while (ms--)
    {
        delay_us(998);
    }
}
