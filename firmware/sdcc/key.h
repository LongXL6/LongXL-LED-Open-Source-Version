#ifndef _KEY_H
#define _KEY_H

#include "types.h"

#define DB_TICKS       3     /* Debounce: N consecutive equal samples */
#define LONG_TICKS     100   /* Held N ticks (~500 ms) enters brightness ramp */
#define RAMP_INTERVAL  3     /* One ramp step every N ticks while ramping */

void KeyInit(void);
void KeyDriver(void);

#endif
