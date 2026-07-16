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

void RampBright(void)
{
	u16 nb = (u16)g_bright + RAMP_STEP;
	if (nb > MAX_BRIGHT)
	{
		nb = MIN_BRIGHT;
	}
	if (nb < MIN_BRIGHT)
	{
		nb = MIN_BRIGHT;
	}
	g_bright = (u8)nb;
	ApplyColor();
}
