#ifndef _KEY_H
#define _KEY_H

#include "config.h"

/* ---- Key timing parameters, in main-loop ticks (~5 ms each) ---- */
#define DB_TICKS       3     /* Debounce: N consecutive equal samples to confirm a level */
#define LONG_TICKS     100   /* Held for N ticks (~500 ms) enters brightness ramp */
#define RAMP_INTERVAL  4     /* 100 perceptual steps x ~20 ms = ~2 s end-to-end */
#define MULTI_CLICK_TICKS 60 /* ~300 ms after the last release closes a click sequence */
#define RAINBOW_MIN_INTERVAL 1  /* Fastest animation: one frame per main-loop tick */
#define RAINBOW_MAX_INTERVAL 20 /* Slowest animation: one frame per ~100 ms */
#define RAINBOW_SPEED_ADJUST 10 /* Change speed every ~50 ms while held */

void KeyInit(void);
void KeyDriver(void);

#endif
