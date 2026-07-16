#ifndef _KEY_H
#define _KEY_H

#include "types.h"

#define DB_TICKS       3     /* Debounce: N consecutive equal samples */
#define LONG_TICKS     100   /* Held N ticks (~500 ms) enters brightness ramp */
#define RAMP_INTERVAL  4     /* 100 perceptual steps x ~20 ms = ~2 s end-to-end */
#define MULTI_CLICK_TICKS 60
#define RAINBOW_MIN_INTERVAL 1
#define RAINBOW_MAX_INTERVAL 20
#define RAINBOW_SPEED_ADJUST 10

void KeyInit(void);
void KeyDriver(void);

#endif
