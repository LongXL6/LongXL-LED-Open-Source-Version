#include "key.h"
#include "ws2812.h"
#include "eeprom.h"

#define KEY_DOWN 0        /* Low level = pressed */

static u8    db_cnt;
static __bit key_stable;  /* Debounced level: 1 = released, 0 = pressed */
static u16   hold_cnt;
static __bit ramping;
static u8    ramp_div;

void KeyInit(void)
{
	P3M0 &= ~0x04;        /* P3.2 quasi-bidirectional input */
	P3M1 &= ~0x04;
	db_cnt     = 0;
	key_stable = 1;
	hold_cnt   = 0;
	ramping    = 0;
	ramp_div   = 0;
}

/* Called every ~5 ms. Short press = next color (fires on release);
   long press = brightness ramp, saved on release. */
void KeyDriver(void)
{
	__bit raw;
	raw = (P3 & 0x04) ? 1 : 0;    /* Read P3.2 */

	if (raw != key_stable)
	{
		if (++db_cnt >= DB_TICKS)
		{
			db_cnt     = 0;
			key_stable = raw;
			if (key_stable == KEY_DOWN)          /* Press confirmed */
			{
				hold_cnt = 0;
				ramping  = 0;
				ramp_div = 0;
			}
			else                                  /* Release confirmed */
			{
				if (ramping)
				{
					SaveSettings();
				}
				else
				{
					NextColor();
					SaveSettings();
				}
				ramping = 0;
			}
		}
	}
	else
	{
		db_cnt = 0;
	}

	if (key_stable == KEY_DOWN)
	{
		if (hold_cnt < 0xFFFF) hold_cnt++;

		if (!ramping)
		{
			if (hold_cnt >= LONG_TICKS)
			{
				ramping  = 1;
				ramp_div = 0;
			}
		}
		else
		{
			if (++ramp_div >= RAMP_INTERVAL)
			{
				ramp_div = 0;
				RampBright();
			}
		}
	}
}
