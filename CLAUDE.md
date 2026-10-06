# TSD Pacer

In-car speed + section computer for classic rally TSD. See `README.md` for the full project description, hardware list, and goals/success criteria — this file is operational context for working in the repo, not a restatement of it.

## Guiding constraint

**Simplicity and speed over architecture. Zero bloat.** This is a single-purpose latency-critical device (render a number fast, update it fast) — not a general app platform. Every abstraction/dependency has to earn its place against that. Concretely this has already ruled out: LVGL (display is driven directly over SPI instead), a multi-component `components/` tree (everything lives in one `main/` component), and premature test/tooling scaffolding (added only when a stage actually needs it, e.g. offline Kalman-filter tuning data, not day one).

## Hardware

- MCU/display: Waveshare ESP32-P4-WiFi6-Touch-LCD-3.5 (ESP32-P4NRW32, 3.5" 320×480 IPS capacitive touch, ST7796 over SPI, FT6336 touch over I2C — touch unused in v1). No native 12V input; fed via a separate buck/LDO stage (see README's Hardware section for the open regulator-feed-point decision).
- GNSS: Quectel LC29H (AA preferred) + dual-band L1/L5 antenna — external, UART.
- Physical section-reset/next buttons — external, wired to the GPIO header (the board's own PWR/RESET/BOOT buttons are not for in-field use).
- Pin assignments are **not yet confirmed** — `docs/pinout.md` is the source of truth once the board's actual schematic is in hand. Don't carry over pin numbers from sibling Waveshare boards (3.4C/4C/X/XC); they're a different SKU.

## Firmware

- **ESP-IDF native** (not Arduino, not PlatformIO), target `esp32p4`, IDF v5.4 (checked out at `~/esp/esp-idf`, `release/v5.4` branch; `source ~/esp/esp-idf/export.sh` to get `idf.py` on PATH). IDF's own tooling runs under Homebrew's `python@3.11` — the system default `python3` was too new (3.14) for IDF's supported range at setup time.
- Chosen over Arduino because ESP32-P4 peripheral coverage there (esp. PCNT, needed only if Goal 3 is built) is still partial; over PlatformIO because its P4 support is a community fork lagging Espressif's own releases.
- Build layout: single `main/` component (`app_main.c`, `display.c/h`, and `gnss.c/h` / `buttons.c/h` / `section.c/h` / `wheel_fusion.c/h` as each stage is built — see below), top-level `CMakeLists.txt` + `sdkconfig.defaults` + `partitions.csv` (simple factory app + nvs, no OTA).

## Build

```
source ~/esp/esp-idf/export.sh
idf.py set-target esp32p4   # first time only
idf.py build
idf.py -p <port> flash monitor
```

## Implementation stages (matches README goal priority)

0. **Bring-up** — blank project builds/flashes; direct SPI write to the ST7796 shows a test pattern. *(current stage — `display.c` has the SPI/DCS-command skeleton in place but pin numbers in `app_config.h` are placeholders (`-1`) and the panel init sequence's orientation bits are unverified; both block actually seeing anything on real hardware until the schematic is confirmed.)*
1. **Goal 1** — `gnss.c` (UART+DMA, LC29H config, RMC/VTG parsing, fix/no-fix state) + digit-cell-only display redraw. Target: ~100 ms fix-to-pixel latency.
2. **Goal 2** — `buttons.c` + `section.c` (distance/time/avg-speed per section, calibration factor persisted in NVS).
3. **Goal 3 (optional/stretch)** — `wheel_fusion.c`, PCNT tick capture + Kalman filter fused with GNSS Doppler. The device is considered complete and shippable at Goal 2; only build this if there's time/interest afterward. Gated by `TSD_ENABLE_WHEEL_FUSION` in `app_config.h`.

## Open decisions (see README's "Open questions" + the risks noted inline in code comments)

- Actual pin assignments (pending schematic).
- LC29H variant on hand (AA vs CA) and its filter/static-hold settings.
- 12V regulator feed point (into USB-C VBUS vs. the AXP2101 battery input).
- Sunlight readability of the 220 cd/m² panel — may need a UI/brightness mitigation.
