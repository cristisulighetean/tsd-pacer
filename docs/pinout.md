# Pin allocation — TBD pending schematic

Source of truth for `main/app_config.h`. Do not fill in numbers from sibling
Waveshare boards (3.4C/4C/X/XC) — confirm against this exact 3.5" SKU's own
schematic first.

| Peripheral | Bus | Status |
|---|---|---|
| LCD (ST7796) | SPI | fixed by board, pins TBD |
| Touch (FT6336) | I2C | fixed by board, unused in v1 |
| ESP32-C6 co-module | SDIO | fixed by board, unused (no networking in scope) |
| TF card | SDIO | fixed by board, unused until SD logging is ever added |
| GNSS (LC29H) UART | UART | TBD — needs a free hardware UART, not UART0 (console) |
| Section buttons | GPIO + interrupt | TBD |
| Wheel-tick sensor (optional, Goal 3) | GPIO → PCNT | TBD, reserve one pin even if unused for now |
