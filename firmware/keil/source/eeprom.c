#include "eeprom.h"
#include "ws2812.h"      /* Needs g_bright / g_color_idx and the default/range macros */

/* IAP commands / control bits */
#define IAP_EN      0x80     /* IAP_CONTR.IAPEN enable bit */
#define IAP_STANDBY 0x00
#define IAP_READ    0x01
#define IAP_WRITE   0x02
#define IAP_ERASE   0x03

/* "Settings valid" marker */
#define SETTINGS_MAGIC 0xA5

/* FOSC in MHz; internal oscillator 35M -> 35. IAP wait states set via IAP_TPS */
#define IAP_FREQ_MHZ   35

/* Shut IAP down and point the address outside the EEPROM area to prevent stray operations */
static void IAP_Idle(void)
{
	IAP_CONTR = 0;
	IAP_CMD   = 0;
	IAP_TRIG  = 0;
	IAP_ADDRH = 0xFF;
	IAP_ADDRL = 0xFF;
}

u8 EEPROM_Read(u16 addr)
{
	u8 dat;
	bit ea;
	ea = EA;             /* Interrupts off during the trigger sequence (0x5A/0xA5)
	                        so it can't be broken; restored afterwards */
	EA = 0;
	IAP_CONTR = IAP_EN;
	IAP_TPS   = IAP_FREQ_MHZ;
	IAP_CMD   = IAP_READ;
	IAP_ADDRH = (u8)(addr >> 8);
	IAP_ADDRL = (u8)addr;
	IAP_TRIG  = 0x5A;
	IAP_TRIG  = 0xA5;
	_nop_();
	dat = IAP_DATA;
	IAP_Idle();
	EA = ea;
	return dat;
}

void EEPROM_Write(u16 addr, u8 dat)
{
	bit ea;
	ea = EA;
	EA = 0;
	IAP_CONTR = IAP_EN;
	IAP_TPS   = IAP_FREQ_MHZ;
	IAP_CMD   = IAP_WRITE;
	IAP_ADDRH = (u8)(addr >> 8);
	IAP_ADDRL = (u8)addr;
	IAP_DATA  = dat;
	IAP_TRIG  = 0x5A;
	IAP_TRIG  = 0xA5;
	_nop_();
	IAP_Idle();
	EA = ea;
}

void EEPROM_Erase(u16 addr)
{
	bit ea;
	ea = EA;
	EA = 0;
	IAP_CONTR = IAP_EN;
	IAP_TPS   = IAP_FREQ_MHZ;
	IAP_CMD   = IAP_ERASE;
	IAP_ADDRH = (u8)(addr >> 8);
	IAP_ADDRL = (u8)addr;
	IAP_TRIG  = 0x5A;
	IAP_TRIG  = 0xA5;
	_nop_();
	IAP_Idle();
	EA = ea;
}

/* Power-up restore: if the magic is valid, read back color/brightness, else use defaults */
void LoadSettings(void)
{
	if(EEPROM_Read(EEPROM_ADDR) == SETTINGS_MAGIC)
	{
		g_color_idx = EEPROM_Read(EEPROM_ADDR + 1);
		g_bright    = EEPROM_Read(EEPROM_ADDR + 2);
		if(g_color_idx >= COLOR_NUM) g_color_idx = DEFAULT_COLOR;
		if(g_bright   <  MIN_BRIGHT) g_bright    = MIN_BRIGHT;
	}
	else
	{
		g_color_idx = DEFAULT_COLOR;
		g_bright    = DEFAULT_BRIGHT;
	}
}

/* Save: erase the sector, then write magic + color + brightness.
   Only called on key release to keep erase/write cycles low. */
void SaveSettings(void)
{
	EEPROM_Erase(EEPROM_ADDR);
	EEPROM_Write(EEPROM_ADDR + 0, SETTINGS_MAGIC);
	EEPROM_Write(EEPROM_ADDR + 1, g_color_idx);
	EEPROM_Write(EEPROM_ADDR + 2, g_bright);
}
