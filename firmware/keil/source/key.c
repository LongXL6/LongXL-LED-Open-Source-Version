#include "key.h"
#include "ws2812.h"
#include "eeprom.h"

sbit KEY1 = P3^2;         /* Button, active low */

#define KEY_DOWN 0        /* Low level = pressed */

static u8  db_cnt;        /* Debounce counter */
static bit key_stable;    /* Debounced stable level: 1 = released, 0 = pressed */
static u16 hold_cnt;      /* Ticks held since the press was confirmed */
static bit ramping;       /* Whether this hold has entered the brightness ramp */
static u8  ramp_div;      /* Ramp step divider counter */

void KeyInit(void)
{
	P3M0 &= ~0x04;        /* P3.2 quasi-bidirectional input (internal weak pull-up) */
	P3M1 &= ~0x04;

	db_cnt     = 0;
	key_stable = 1;       /* Assume released at power-up */
	hold_cnt   = 0;
	ramping    = 0;
	ramp_div   = 0;
}

/* Called from the main loop every ~5 ms.
   Short press (released before the threshold) = next color (fires on release,
   so it can be told apart from a long press);
   Long press (past the threshold) = continuous brightness ramp, stops at the
   current value on release; settings are saved on release. */
void KeyDriver(void)
{
	bit raw = KEY1;

	/* --- Debounce: level flips only after DB_TICKS consecutive mismatches --- */
	if(raw != key_stable)
	{
		if(++db_cnt >= DB_TICKS)
		{
			db_cnt     = 0;
			key_stable = raw;
			if(key_stable == KEY_DOWN)          /* Press confirmed */
			{
				hold_cnt = 0;
				ramping  = 0;
				ramp_div = 0;
			}
			else                                 /* Release confirmed */
			{
				if(ramping)
				{
					SaveSettings();              /* Long press ended: save brightness */
				}
				else
				{
					NextColor();                 /* Short press: next color */
					SaveSettings();              /* Save color */
				}
				ramping = 0;
			}
		}
	}
	else
	{
		db_cnt = 0;
	}

	/* --- Stable pressed: count ticks; past the threshold, enter and run the ramp --- */
	if(key_stable == KEY_DOWN)
	{
		if(hold_cnt < 0xFFFF) hold_cnt++;

		if(!ramping)
		{
			if(hold_cnt >= LONG_TICKS)
			{
				ramping  = 1;
				ramp_div = 0;
			}
		}
		else
		{
			if(++ramp_div >= RAMP_INTERVAL)
			{
				ramp_div = 0;
				RampBright();
			}
		}
	}
}
