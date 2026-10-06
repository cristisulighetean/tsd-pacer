#pragma once

// Goal 3 (wheel-tick + Kalman fusion) is optional/stretch — leave off until
// Stage 3 actually starts, and wheel_fusion.c/.h won't exist until then.
#define TSD_ENABLE_WHEEL_FUSION 0

// Pin map — all TBD pending the board's actual schematic (see docs/pinout.md).
// Do not guess sibling-board (3.4C/4C/X/XC) pin numbers for this 3.5" SKU.
#define TSD_PIN_LCD_CS   (-1)
#define TSD_PIN_LCD_DC   (-1)
#define TSD_PIN_LCD_RST  (-1)
#define TSD_PIN_LCD_BL   (-1)
