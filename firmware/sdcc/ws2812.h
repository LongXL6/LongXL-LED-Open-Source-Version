#ifndef _WS2812_H
#define _WS2812_H

#include "types.h"

#define LedNum    50
#define COLOR_NUM 7

#define MIN_BRIGHT     8
#define MAX_BRIGHT     255
#define DEFAULT_BRIGHT 26
#define DEFAULT_COLOR  0
#define RAMP_LEVELS    100

extern u8 g_bright;
extern u8 g_color_idx;

void LedRefresh(void);
void WS2812_Init(void);
void SetLedColor(u8 ledx, u8 r, u8 g, u8 b);
void ApplyColor(void);
void NextColor(void);
u8 RampBright(u8 up);
void BeginBrightRamp(void);
void SignalBrightLimit(void);
void RainbowStep(void);

#endif
