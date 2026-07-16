#ifndef __WS2812_H
#define __WS2812_H

#include "config.h"

#define LedNum  50       /* Number of LEDs on the strip */
#define COLOR_NUM 7      /* Entries in the color table (keep in sync with color_tab) */

/* ---- Brightness model: adjustable at runtime, every color shares one value ---- */
#define MIN_BRIGHT     8     /* Lower limit (never ramps to 0, so it never looks dead) */
#define MAX_BRIGHT     255   /* Upper limit (lower this to cap current draw) */
#define DEFAULT_BRIGHT 26    /* ~10%, used on first power-up / invalid EEPROM */
#define DEFAULT_COLOR  0     /* Default color: red */
#define RAMP_STEP      2     /* Brightness increment per ramp step while held */

extern u8 g_bright;      /* Current brightness 0..255 (actual value of each lit channel) */
extern u8 g_color_idx;   /* Current color index 0..COLOR_NUM-1 */

void LedRefresh(void);
void WS2812_Init(void);
void SetLedColor(u8 ledx,u8 r,u8 g,u8 b);
void SetLedHSVColor(u8 ledx,u16 h,u8 s,u8 v);
void HSVtoRGB(u8 *r, u8 *g, u8 *b, u16 h, u8 s, u8 v);

void ApplyColor(void);   /* Refresh whole strip from g_color_idx + g_bright */
void NextColor(void);    /* Advance to next color (wraps, never out of range) and refresh */
void RampBright(void);   /* Long-press ramp: one brightness step (wraps at top) and refresh */

#endif
