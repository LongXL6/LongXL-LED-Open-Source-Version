#ifndef _STC8G_REG_H
#define _STC8G_REG_H

/* Only the STC8G special-function registers this project actually uses (SDCC syntax) */
__sfr  __at (0xB0) P3;            /* Button on P3.2, read via (P3 & 0x04) */
__sbit __at (0xB3) WS2812_IO;    /* P3.3, bit address 0xB3 (timing critical, bit-addressed) */
__sfr  __at (0xB1) P3M1;
__sfr  __at (0xB2) P3M0;
__sbit __at (0xAF) EA;           /* IE.7 */

__sfr  __at (0xC2) IAP_DATA;
__sfr  __at (0xC3) IAP_ADDRH;
__sfr  __at (0xC4) IAP_ADDRL;
__sfr  __at (0xC5) IAP_CMD;
__sfr  __at (0xC6) IAP_TRIG;
__sfr  __at (0xC7) IAP_CONTR;
__sfr  __at (0xF5) IAP_TPS;

#endif
