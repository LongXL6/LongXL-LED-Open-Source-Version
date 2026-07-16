#include "eeprom.h"
#include "ws2812.h"      /* g_bright / g_color_idx and the range/default macros */

#define IAP_EN       0x80
#define IAP_READ     0x01
#define IAP_WRITE    0x02
#define IAP_ERASE    0x03
#define SETTINGS_MAGIC 0xA5
#define IAP_FREQ_MHZ 35     /* FOSC (MHz), internal 35M */

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
	__bit ea;
	ea = EA;
	EA = 0;                 /* Interrupts off during the trigger sequence */
	IAP_CONTR = IAP_EN;
	IAP_TPS   = IAP_FREQ_MHZ;
	IAP_CMD   = IAP_READ;
	IAP_ADDRH = (u8)(addr >> 8);
	IAP_ADDRL = (u8)addr;
	IAP_TRIG  = 0x5A;
	IAP_TRIG  = 0xA5;
	__asm nop __endasm;
	dat = IAP_DATA;
	IAP_Idle();
	EA = ea;
	return dat;
}

void EEPROM_Write(u16 addr, u8 dat)
{
	__bit ea;
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
	__asm nop __endasm;
	IAP_Idle();
	EA = ea;
}

void EEPROM_Erase(u16 addr)
{
	__bit ea;
	ea = EA;
	EA = 0;
	IAP_CONTR = IAP_EN;
	IAP_TPS   = IAP_FREQ_MHZ;
	IAP_CMD   = IAP_ERASE;
	IAP_ADDRH = (u8)(addr >> 8);
	IAP_ADDRL = (u8)addr;
	IAP_TRIG  = 0x5A;
	IAP_TRIG  = 0xA5;
	__asm nop __endasm;
	IAP_Idle();
	EA = ea;
}

void LoadSettings(void)
{
	if (EEPROM_Read(EEPROM_ADDR) == SETTINGS_MAGIC)
	{
		g_color_idx = EEPROM_Read(EEPROM_ADDR + 1);
		g_bright    = EEPROM_Read(EEPROM_ADDR + 2);
		if (g_color_idx >= COLOR_NUM) g_color_idx = DEFAULT_COLOR;
		if (g_bright   <  MIN_BRIGHT) g_bright    = MIN_BRIGHT;
	}
	else
	{
		g_color_idx = DEFAULT_COLOR;
		g_bright    = DEFAULT_BRIGHT;
	}
}

void SaveSettings(void)
{
	EEPROM_Erase(EEPROM_ADDR);
	EEPROM_Write(EEPROM_ADDR + 0, SETTINGS_MAGIC);
	EEPROM_Write(EEPROM_ADDR + 1, g_color_idx);
	EEPROM_Write(EEPROM_ADDR + 2, g_bright);
}
