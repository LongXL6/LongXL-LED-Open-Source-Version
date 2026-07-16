#include "ws2812.h"

sbit WS2812_IO = P3^3;                  /* WS2812 data pin */
u8 xdata LED_BYTE_Buffer[LedNum][3];    /* TX buffer, 3 bytes per LED, order G R B */

u8 g_bright    = DEFAULT_BRIGHT;        /* Current brightness 0..255 */
u8 g_color_idx = DEFAULT_COLOR;         /* Current color index */

/* 7 base colors stored as on/off flags (R,G,B); actual channel value =
   flag ? g_bright : 0, so every color always uses the same brightness value. */
static const u8 color_tab[COLOR_NUM][3] = {
	{1,0,0},   /* red */
	{0,1,0},   /* green */
	{0,0,1},   /* blue */
	{1,0,1},   /* magenta */
	{1,1,0},   /* yellow */
	{0,1,1},   /* cyan */
	{1,1,1},   /* white */
};

/* HSV to RGB. *r *g *b: output pointers; h hue 0~359, s saturation 0~100, v value 0~100 */
void HSVtoRGB(u8 *r, u8 *g, u8 *b, u16 h, u8 s, u8 v)
{
	u8  i;
	u16  difs;
	float RGB_min,RGB_max,RGB_Adj;

	RGB_max = v*2.55f;
	RGB_min = RGB_max*(100 - s) / 100.0f;

	i = h / 60;
	difs = h % 60;
	RGB_Adj = (RGB_max - RGB_min)*difs / 60.0f;

	switch (i) {
	case 0:
		*r = RGB_max;
		*g = RGB_min + RGB_Adj;
		*b = RGB_min;
		break;
	case 1:
		*r = RGB_max - RGB_Adj;
		*g = RGB_max;
		*b = RGB_min;
		break;
	case 2:
		*r = RGB_min;
		*g = RGB_max;
		*b = RGB_min + RGB_Adj;
		break;
	case 3:
		*r = RGB_min;
		*g = RGB_max - RGB_Adj;
		*b = RGB_max;
		break;
	case 4:
		*r = RGB_min + RGB_Adj;
		*g = RGB_min;
		*b = RGB_max;
		break;
	default:
		*r = RGB_max;
		*g = RGB_min;
		*b = RGB_max - RGB_Adj;
		break;
	}
}

/* Set one LED's color. ledx: position 0~N, r/g/b: 0~255.
   Buffer is stored in the WS2812's GRB byte order. */
void SetLedColor(u8 ledx,u8 r,u8 g,u8 b)
{
	LED_BYTE_Buffer[ledx][0] = g;
	LED_BYTE_Buffer[ledx][1] = r;
	LED_BYTE_Buffer[ledx][2] = b;
}

/* Set one LED's color via HSV */
void SetLedHSVColor(u8 ledx,u16 h,u8 s,u8 v)
{
	HSVtoRGB(&LED_BYTE_Buffer[ledx][1], &LED_BYTE_Buffer[ledx][0], &LED_BYTE_Buffer[ledx][2], h, s, v);
}

/* Shift the buffer out bit by bit.
   Bit timing is hand-tuned for 35 MHz — do NOT change the nop counts. */
void LedRefresh(void)
{
	u8 i,j,k,dat;
	EA = 0;                           /* Disable interrupts so timing can't be broken */
	for(k = 0; k < LedNum; k++)       /* Every LED */
	{
		for(j = 0; j < 3; j++)        /* Bytes G R B */
		{
			dat = LED_BYTE_Buffer[k][j];
			for(i = 0; i < 8; i++)    /* 8 bits, MSB first */
			{
				WS2812_IO = 1;
				_nop_();_nop_();_nop_();_nop_();_nop_();
				_nop_();_nop_();_nop_();_nop_();
				WS2812_IO = (dat & 0x80);
				_nop_();_nop_();_nop_();_nop_();_nop_();
				_nop_();_nop_();_nop_();_nop_();_nop_();
				_nop_();_nop_();
				dat <<= 1;
				WS2812_IO = 0;
				_nop_();_nop_();_nop_();_nop_();
				_nop_();
			}
		}
	}
	EA = 1;
}

/* WS2812 init */
void WS2812_Init(void)
{
	u8 i;

	WS2812_IO = 0;
	P3M0 &= 0xF7;        /* P3.3 quasi-bidirectional (note: not push-pull) */
	P3M1 &= 0xF7;

	for(i = 0; i < LedNum; i++)   /* Start all-off */
	{
		SetLedColor(i,0,0,0);
	}
	LedRefresh();
}

/* Fill the whole strip from current color index + current brightness, then refresh */
void ApplyColor(void)
{
	u8 i;
	u8 r = color_tab[g_color_idx][0] ? g_bright : 0;
	u8 g = color_tab[g_color_idx][1] ? g_bright : 0;
	u8 b = color_tab[g_color_idx][2] ? g_bright : 0;
	for(i = 0; i < LedNum; i++)
	{
		SetLedColor(i, r, g, b);
	}
	LedRefresh();
}

/* Advance to the next color; wraps around, never out of range */
void NextColor(void)
{
	g_color_idx++;
	if(g_color_idx >= COLOR_NUM)
	{
		g_color_idx = 0;
	}
	ApplyColor();
}

/* Long-press ramp: brightness one step up; past the upper limit it wraps
   back to the lower limit, then refresh.
   (Alternative feel: ping-pong at the top — would need to reverse direction
   there; per the requirement this uses wrap.) */
void RampBright(void)
{
	u16 nb = (u16)g_bright + RAMP_STEP;
	if(nb > MAX_BRIGHT)
	{
		nb = MIN_BRIGHT;
	}
	if(nb < MIN_BRIGHT)
	{
		nb = MIN_BRIGHT;
	}
	g_bright = (u8)nb;
	ApplyColor();
}
