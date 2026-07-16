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

/* Gamma 2.2: equal table-index steps look approximately equally bright. */
static const u8 bright_tab[RAMP_LEVELS + 1] = {
	8,8,8,8,8,8,9,9,9,9,10,10,10,11,11,12,12,13,14,14,
	15,16,17,18,19,20,21,22,23,24,25,27,28,30,31,33,34,36,37,39,
	41,43,45,47,49,51,53,55,57,59,62,64,67,69,72,74,77,80,83,85,
	88,91,94,97,101,104,107,110,114,117,121,124,128,132,135,139,143,147,151,155,
	159,163,168,172,176,181,185,190,194,199,204,209,214,219,224,229,234,239,244,250,255
};
static u8 bright_level;

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

/* Gamma-corrected long-press ramp. The key driver controls direction and
   stops at either limit.
   (Alternative feel: ping-pong at the top — would need to reverse direction
   there; per the requirement this uses wrap.) */
void BeginBrightRamp(void)
{
	u8 i;
	for(i = 0; i < RAMP_LEVELS && bright_tab[i] < g_bright; i++);
	bright_level = i;
}

u8 RampBright(u8 up)
{
	if(up && bright_level < RAMP_LEVELS) bright_level++;
	if(!up && bright_level > 0) bright_level--;
	g_bright = bright_tab[bright_level];
	ApplyColor();
	return (up && bright_level >= RAMP_LEVELS) || (!up && bright_level == 0);
}

void SignalBrightLimit(void)
{
	u8 n, i;
	for(n = 0; n < 2; n++)
	{
		for(i = 0; i < 4 && i < LedNum; i++) SetLedColor(i, 0, 0, 0);
		LedRefresh();
		delay_ms(100);
		ApplyColor();
		delay_ms(100);
	}
}

static void Wheel(u8 p, u8 *r, u8 *g, u8 *b)
{
	if(p < 85) { *r = 255 - p * 3; *g = p * 3; *b = 0; }
	else if(p < 170) { p -= 85; *r = 0; *g = 255 - p * 3; *b = p * 3; }
	else { p -= 170; *r = p * 3; *g = 0; *b = 255 - p * 3; }
}

void RainbowStep(void)
{
	static u8 phase;
	u8 i, r, g, b;
	for(i = 0; i < LedNum; i++)
	{
		Wheel((u8)(phase + ((u16)i * 256u / LedNum)), &r, &g, &b);
		SetLedColor(i, r, g, b);
	}
	phase++;
	LedRefresh();
}
