#include "config.h"
#include "ws2812.h"
#include "key.h"
#include "eeprom.h"

void main(void)
{
	KeyInit();
	WS2812_Init();
	LoadSettings();      /* Restore last color/brightness from EEPROM (defaults if invalid) */
	ApplyColor();        /* Show the restored color/brightness */

	while(1)
	{
		KeyDriver();
		delay_ms(5);
	}
}
