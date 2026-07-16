#include "key.h"
#include "ws2812.h"
#include "eeprom.h"

#define KEY_DOWN 0

static u8 db_cnt, ramp_div;
static u16 hold_cnt;
static __bit key_stable, ramping, ramp_up, at_limit;
static u8 click_count, click_timer;
static __bit rainbow_mode, rainbow_faster;
static u8 rainbow_interval, rainbow_frame_div, rainbow_adjust_div;

void KeyInit(void)
{
	P3M0 &= ~0x04;
	P3M1 &= ~0x04;
	db_cnt = hold_cnt = ramp_div = 0;
	key_stable = 1;
	ramping = at_limit = 0;
	ramp_up = (g_bright < MAX_BRIGHT);
	click_count = click_timer = 0;
	rainbow_mode = 0;
	rainbow_faster = 1;
	rainbow_interval = 10;
	rainbow_frame_div = rainbow_adjust_div = 0;
}

void KeyDriver(void)
{
	__bit raw;
	u8 i;
	raw = (P3 & 0x04) ? 1 : 0;

	if(raw != key_stable)
	{
		if(++db_cnt >= DB_TICKS)
		{
			db_cnt = 0;
			key_stable = raw;
			if(key_stable == KEY_DOWN)
			{
				hold_cnt = 0;
				ramping = at_limit = 0;
				ramp_div = rainbow_adjust_div = 0;
			}
			else
			{
				if(ramping)
				{
					if(rainbow_mode) rainbow_faster = !rainbow_faster;
					else SaveSettings();
				}
				else
				{
					click_count++;
					click_timer = 0;
					if(!rainbow_mode && click_count >= 5)
					{
						rainbow_mode = 1;
						click_count = 0;
						rainbow_frame_div = 0;
						RainbowStep();
					}
					else if(rainbow_mode && click_count >= 4)
					{
						rainbow_mode = 0;
						click_count = 0;
						ApplyColor();
					}
				}
				ramping = 0;
			}
		}
	}
	else db_cnt = 0;

	if(key_stable == KEY_DOWN)
	{
		if(hold_cnt < 0xFFFF) hold_cnt++;
		if(!ramping && hold_cnt >= LONG_TICKS)
		{
			click_count = 0;
			ramping = 1;
			if(!rainbow_mode)
			{
				if(g_bright >= MAX_BRIGHT) ramp_up = 0;
				else if(g_bright <= MIN_BRIGHT) ramp_up = 1;
				BeginBrightRamp();
			}
		}
		else if(ramping && rainbow_mode)
		{
			if(++rainbow_adjust_div >= RAINBOW_SPEED_ADJUST)
			{
				rainbow_adjust_div = 0;
				if(rainbow_faster && rainbow_interval > RAINBOW_MIN_INTERVAL) rainbow_interval--;
				if(!rainbow_faster && rainbow_interval < RAINBOW_MAX_INTERVAL) rainbow_interval++;
			}
		}
		else if(ramping && !at_limit && ++ramp_div >= RAMP_INTERVAL)
		{
			ramp_div = 0;
			if(RampBright(ramp_up))
			{
				SignalBrightLimit();
				ramp_up = !ramp_up;
				at_limit = 1;
			}
		}
	}
	else if(click_count && ++click_timer >= MULTI_CLICK_TICKS)
	{
		if(!rainbow_mode)
		{
			for(i = 0; i < click_count; i++) NextColor();
			SaveSettings();
		}
		click_count = click_timer = 0;
	}

	if(rainbow_mode && ++rainbow_frame_div >= rainbow_interval)
	{
		rainbow_frame_div = 0;
		RainbowStep();
	}
}
