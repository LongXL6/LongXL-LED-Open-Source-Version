#ifndef _KEY_H
#define _KEY_H

#include "config.h"

/* ---- Key timing parameters, in main-loop ticks (~5 ms each) ---- */
#define DB_TICKS       3     /* Debounce: N consecutive equal samples to confirm a level */
#define LONG_TICKS     100   /* Held for N ticks (~500 ms) enters brightness ramp */
#define RAMP_INTERVAL  3     /* While ramping, one brightness step every N ticks */

void KeyInit(void);
void KeyDriver(void);

#endif
