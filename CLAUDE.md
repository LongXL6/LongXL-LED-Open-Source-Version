# ws2812-stc8g1k08a

One-button WS2812 LED strip controller firmware for the STC8G1K08A
(8-pin, 1T 8051). Short press = next of 7 colors; long press = brightness
ramp; settings persist in on-chip EEPROM.

## Stack / layout

- `firmware/keil/` — original Keil C51 build (project/ + source/). Cannot be
  built on macOS; do not "verify" it by compiling — verify by comment-stripped
  diff instead.
- `firmware/sdcc/` — SDCC port, build with `cd firmware/sdcc && ./build.sh`
  (needs `brew install sdcc`). The build is reproducible; the hex must stay
  byte-identical unless code is intentionally changed.
- `prebuilt/ws2812_stc8g1k08a_35mhz.hex` — the single flashable hex (SDCC
  build of the current sources). Flashing is done via STC-ISP (Windows) or
  stcgal, always with IRC = 35.000 MHz (see docs/images/stc-isp-settings.png).
  The old Keil-built hex from `output/` was excluded on purpose: its 2025-03-04
  build log shows it predates the EEPROM/settings-memory feature.

## Hard constraints

- `LedRefresh()` in both trees is timing-critical, hand-tuned for a 35 MHz
  internal RC clock. Never reorder code or change nop counts there.
- Keil tree drives P3.3 quasi-bidirectional; SDCC tree drives it push-pull.
  This difference is intentional and documented in README.md.
- Docs are bilingual: any change to README.md must be mirrored in
  README.zh-CN.md.

## Deployment

None (bare-metal firmware). "Release" = tag + attach prebuilt hexes
(see PUBLISHING.md).
