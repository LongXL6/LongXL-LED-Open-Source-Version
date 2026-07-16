#ifndef _EEPROM_H
#define _EEPROM_H

#include "config.h"

/* The STC8G EEPROM/DataFlash is a separate address space starting at 0x0000
   (not an absolute address in code flash). Sector 0 (0x0000~0x01FF) stores
   the settings. Erase granularity is one 512-byte sector. */
#define EEPROM_ADDR   0x0000

/* STC8G IAP low-level read/write/erase (erase works on 512-byte sectors) */
u8   EEPROM_Read(u16 addr);
void EEPROM_Write(u16 addr, u8 dat);
void EEPROM_Erase(u16 addr);

/* Application layer: restore/save current color index g_color_idx and brightness g_bright */
void LoadSettings(void);
void SaveSettings(void);

#endif
