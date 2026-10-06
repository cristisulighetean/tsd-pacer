# TSD Pacer: Rally TSD Speed & Section Computer

## Project description

A compact in-car speed and section computer for classic rally TSD. An ESP32 reads a Quectel LC29H dual-band GNSS module at 10 Hz and shows the car's current speed as accurately and quickly as possible on a bright, driver-friendly display. The speed comes from the receiver's Doppler velocity measurement rather than position changes, so it stays steady and responds within about 0.1 s. Secondary functions are resettable sections that track distance, time, and average speed, operated with physical buttons.

## Hardware

- **MCU/display board:** [Waveshare ESP32-P4-WiFi6-Touch-LCD-3.5](https://www.waveshare.com/product/iot-communication/short-range-wireless/bluetooth/esp32-p4-wifi6-touch-lcd-3.5.htm) — ESP32-P4NRW32 (RISC-V dual-core), 32MB PSRAM / 16MB flash, Wi-Fi6+BLE5 via onboard ESP32-C6 co-module (SDIO), 3.5" 320×480 IPS capacitive touch (ST7796 + FT6336), 16-bit 2.54mm GPIO header, USB-C (prog + OTG), TF slot, AXP2101 PMIC w/ Li-ion charging. Onboard camera unused for this project.
- Quectel LC29H (AA preferred; CA only if verified) — external module, not onboard
- Dual-band L1/L5 active antenna
- Physical trigger button(s), external — section reset/next, wired to GPIO header (board's own PWR/RESET/BOOT buttons are not for in-field use)
- Protected 12 V supply + regulator — board has no native 12 V input (native inputs are USB-C 5 V or 3.7 V Li-ion via AXP2101); needs a buck/LDO stage down to 5 V (USB-C) or board's battery input

## GNSS module options

Two ways to get wheel-speed-aided speed: fuse it myself on the ESP32 (Goal 3), or use an LC29H variant with built-in dead reckoning (DR). Per distributor listings (verify against the Quectel datasheet and DR application note):

| Variant | Function | Fit for this project |
|---|---|---|
| LC29H-AA | Standard dual-band GNSS | Baseline. Simplest, pairs with own Kalman fusion |
| LC29H-CA | DR (integrated IMU) | Candidate. Built-in fusion of GNSS, IMU and wheel data; bridges outages |

DR pros:
- Continuous output through tunnels, trees, and outages, with fusion tuned by the manufacturer
- Integrated IMU and wheel-speed handling, less code on the ESP32

DR cons and things to verify:
- Is the RMC/VTG speed still the raw Doppler speed, or does DR alter or smooth it? This could conflict with Goal 1's ~100 ms responsiveness.
- Does the 10 Hz output rate still apply in DR mode?
- Wheel input interface: signal type (pulse/odometer), scale-factor setup, gear/reverse signal, and whether it suits a classic car with a single tick source
- IMU mounting orientation and calibration requirements inside the car
- Fusion is a black box: harder to tune, log, and debug than my own Kalman filter
- Hardware availability and cost of CA breakout boards

Decision path: start with AA plus own fusion. Buy a CA only if the datasheet and application note confirm that raw Doppler speed stays available and wheel input fits. Ideally test both against the same reference route.

## Goal 1 (top priority): best, most current speed

- Use Doppler-derived speed from RMC/VTG at 10 Hz. Position accuracy doesn't matter.
- Enable all constellations, use a good antenna position, and monitor satellite count and HDOP.
- Use a high baud rate, enable only the needed sentences, parse on arrival, and redraw immediately.
- Check the module for speed filtering, smoothing, or static-hold settings, and keep them minimal.
- Show speed only with a valid fix, and show a clear "no fix" state otherwise.

**Success:** the display updates within ~100 ms of each fix, holds steady at constant speed, and matches a reference (calibrated speedometer or timed measured distance).

## Goal 2: resettable sections

- Each section has its own distance, elapsed time, and average speed, with a reset button.
- Distance is speed integrated over time, ignoring speed below a small threshold when stopped.
- Add a calibration factor so distance can be matched to the organizer's measurement.

**Success:** a known-length test route matches within tolerance.

## Goal 3 (optional/stretch): wheel-speed tick sensor + Kalman fusion

Optional — the device is considered a complete, shippable pacer at Goal 2 even if this is never built. Pursue only if time/interest remain afterward.

Add a wheel-speed tick sensor (e.g. Hall/reluctor pickup on the gearbox, driveshaft, or wheel) and fuse it with the GNSS Doppler speed in a Kalman filter.

- Ticks arrive far faster than 10 Hz, so they give a smooth speed estimate *between* GNSS fixes, and keep speed and distance going through tunnels, trees, and short outages.
- GNSS Doppler is the absolute reference. The filter uses it to continuously estimate the wheel scale factor (distance per tick), which absorbs tyre size, wear, pressure, and load changes.
- Candidate state: `[speed, acceleration, scale_factor]` (optionally a tick-rate bias). Predict on each tick window, update on each GNSS fix.
- Measurement noise for GNSS should come from HDOP, satellite count, and speed-accuracy indicators; wheel noise from tick quantization at low speed.
- Handle wheel slip, lock-up, and gravel: reject wheel updates that disagree strongly with GNSS.
- Hardware: ESP32 pulse capture via PCNT or interrupt with timestamps, plus input conditioning and protection for a noisy 12 V automotive signal.
- Fused speed also improves section distance (Goal 2), since distance can be integrated at tick rate.

**Constraint from Goal 1:** the filter must not add lag or over-smooth. Tune it so the displayed speed stays within ~100 ms of the raw Doppler response on real accelerations, and keep a raw-Doppler-only display mode to compare against.

**Success:** fused speed is no less accurate than Doppler alone in open sky, bridges outages of several seconds with small drift, and the estimated scale factor converges and stays stable.

## Later

- Target speed with early/late seconds
- SD card logging (include raw ticks and raw Doppler so the filter can be tuned offline)
- GNSS-synced clock

## Research

- [ ] [On the Relativistic Doppler Effect for Precise Velocity Determination using GPS](https://www.researchgate.net/publication/225359469_Short_Note_On_the_Relativistic_Doppler_Effect_for_Precise_Velocity_Determination_using_GPS)
- [ ] GNSS/odometer sensor fusion with Kalman filters (scale-factor estimation, outage bridging)
- [ ] Wheel sensor options and placement for a classic car
- [ ] Quectel LC29H(BA, CA, DA) DR & RTK Application Note and LC29H series GNSS protocol specification (DR setup, wheel-tick input, speed output behavior)
- [ ]

## Open questions

- Wheel sensor type and mounting point (wheel, gearbox output, driveshaft) and ticks per revolution
- With a wheel sensor, is LC29H-CA (dead-reckoning) still needed, or is the AA plus own fusion enough? (see GNSS module options)
- Read the LC29H DR & RTK application note and GNSS protocol spec: DR wheel-tick input, output sentences and rates in DR mode
- LC29H-AA vs CA: confirm which is on hand and what speed-filter/static-hold settings it exposes
- Display choice and brightness/sunlight readability
- Button layout: section reset, next section, calibration entry
