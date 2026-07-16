#include "ws2812.h"

__xdata u8 LED_BYTE_Buffer[LedNum][3];   /* TX buffer, order G R B */

u8 g_bright    = DEFAULT_BRIGHT;
u8 g_color_idx = DEFAULT_COLOR;

/* 7 base colors as on/off flags (R,G,B); actual channel value = flag ? g_bright : 0 */
static const __code u8 color_tab[COLOR_NUM][3] = {
	{1,0,0},   /* red */
	{0,1,0},   /* green */
	{0,0,1},   /* blue */
	{1,0,1},   /* magenta */
	{1,1,0},   /* yellow */
	{0,1,1},   /* cyan */
	{1,1,1},   /* white */
};

static const __code u8 bright_tab[RAMP_LEVELS + 1] = {
	8,8,8,8,8,8,9,9,9,9,10,10,10,11,11,12,12,13,14,14,
	15,16,17,18,19,20,21,22,23,24,25,27,28,30,31,33,34,36,37,39,
	41,43,45,47,49,51,53,55,57,59,62,64,67,69,72,74,77,80,83,85,
	88,91,94,97,101,104,107,110,114,117,121,124,128,132,135,139,143,147,151,155,
	159,163,168,172,176,181,185,190,194,199,204,209,214,219,224,229,234,239,244,250,255
};
static u8 bright_level;

void SetLedColor(u8 ledx, u8 r, u8 g, u8 b)
{
	LED_BYTE_Buffer[ledx][0] = g;
	LED_BYTE_Buffer[ledx][1] = r;
	LED_BYTE_Buffer[ledx][2] = b;
}

/* Shift out bit by bit, fully in inline assembly, so timing is deterministic
   (tuned for the internal 35 MHz oscillator).
   Structure: for each bit, rlc pre-fetches the MSB into C, then the pin goes
   high; during the high phase there are only nops, so T0H/T1H are set purely
   by the nop counts and are immune to C code generation.
   Targets (35 MHz, 28.6 ns/clock): T0H≈12clk(~340ns) T1H≈24clk(~690ns)
   T1L≈17clk(~490ns) T0L≈29clk(~830ns), all well inside the WS2812B spec.
   Sends LedNum*3 = 150 bytes total. */
void LedRefresh(void)
{
	EA = 0;
	__asm
		mov   dptr, #_LED_BYTE_Buffer
		mov   r7, #150               ; 50 LEDs * 3 bytes
	00081$:
		movx  a, @dptr               ; fetch one byte
		inc   dptr
		mov   r6, #8                 ; 8 bits, MSB first
	00082$:
		rlc   a                      ; MSB -> C (computed before the pin goes high)
		setb  _WS2812_IO             ; rising edge
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop                          ; N0=10 -> T0H
		mov   _WS2812_IO, c          ; a '0' bit goes low here; a '1' bit stays high
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop                          ; N1=10 -> sets T1H
		clr   _WS2812_IO             ; a '1' bit goes low here
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop                          ; N2=10 -> T1L
		djnz  r6, 00082$
		djnz  r7, 00081$
	__endasm;
	EA = 1;
}

void WS2812_Init(void)
{
	u8 i;
	WS2812_IO = 0;
	P3M0 |= 0x08;        /* P3.3 push-pull output: strong drive, sharper edges,
	                        better when feeding several LEDs in parallel */
	P3M1 &= ~0x08;
	for (i = 0; i < LedNum; i++)
	{
		SetLedColor(i, 0, 0, 0);
	}
	LedRefresh();
}

void ApplyColor(void)
{
	u8 i;
	u8 r = color_tab[g_color_idx][0] ? g_bright : 0;
	u8 g = color_tab[g_color_idx][1] ? g_bright : 0;
	u8 b = color_tab[g_color_idx][2] ? g_bright : 0;
	for (i = 0; i < LedNum; i++)
	{
		SetLedColor(i, r, g, b);
	}
	LedRefresh();
}

void NextColor(void)
{
	g_color_idx++;
	if (g_color_idx >= COLOR_NUM)
	{
		g_color_idx = 0;
	}
	ApplyColor();
}

void BeginBrightRamp(void)
{
	u8 i;
	for (i = 0; i < RAMP_LEVELS && bright_tab[i] < g_bright; i++);
	bright_level = i;
}

u8 RampBright(u8 up)
{
	if (up && bright_level < RAMP_LEVELS) bright_level++;
	if (!up && bright_level > 0) bright_level--;
	g_bright = bright_tab[bright_level];
	ApplyColor();
	return (up && bright_level >= RAMP_LEVELS) || (!up && bright_level == 0);
}

void SignalBrightLimit(void)
{
	u8 n, i;
	for (n = 0; n < 2; n++)
	{
		for (i = 0; i < 4 && i < LedNum; i++) SetLedColor(i, 0, 0, 0);
		LedRefresh();
		delay_ms(100);
		ApplyColor();
		delay_ms(100);
	}
}

static void Wheel(u8 p, u8 *r, u8 *g, u8 *b)
{
	if (p < 85) { *r = 255 - p * 3; *g = p * 3; *b = 0; }
	else if (p < 170) { p -= 85; *r = 0; *g = 255 - p * 3; *b = p * 3; }
	else { p -= 170; *r = p * 3; *g = 0; *b = 255 - p * 3; }
}

void RainbowStep(void)
{
	static u8 phase;
	u8 i, r, g, b;
	for (i = 0; i < LedNum; i++)
	{
		Wheel((u8)(phase + ((u16)i * 256u / LedNum)), &r, &g, &b);
		SetLedColor(i, r, g, b);
	}
	phase++;
	LedRefresh();
}
