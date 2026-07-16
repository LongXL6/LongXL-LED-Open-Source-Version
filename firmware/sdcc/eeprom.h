#ifndef _EEPROM_H
#define _EEPROM_H

#include "types.h"

/* The STC8G EEPROM is a separate address space starting at 0x0000
   (not an absolute address in code flash). Sector 0 (0x0000~0x01FF)
   stores the settings. */
#define EEPROM_ADDR   0x0000

u8   EEPROM_Read(u16 addr);
void EEPROM_Write(u16 addr, u8 dat);
void EEPROM_Erase(u16 addr);
void LoadSettings(void);
void SaveSettings(void);

#endif
