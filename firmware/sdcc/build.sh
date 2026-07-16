#!/bin/sh
# Build the WS2812 STC8G1K08A firmware with SDCC (internal 35 MHz oscillator).
# Requires: sdcc (macOS: brew install sdcc | Debian/Ubuntu: apt install sdcc)
set -e
cd "$(dirname "$0")"

rm -f ./*.rel ./*.ihx ./*.lst ./*.rst ./*.sym ./*.asm ./*.map ./*.mem ./*.lk 2>/dev/null || true

for f in main ws2812 key eeprom delay; do
    sdcc -c -mmcs51 "$f.c"
done

sdcc -mmcs51 --xram-size 1024 --iram-size 256 --code-size 0x2000 \
     main.rel ws2812.rel key.rel eeprom.rel delay.rel \
     -o WS2812_STC8G1K08A.ihx

packihx WS2812_STC8G1K08A.ihx > WS2812_STC8G1K08A.hex

echo "==> Done: WS2812_STC8G1K08A.hex"
ls -l WS2812_STC8G1K08A.hex
