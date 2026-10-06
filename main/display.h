#pragma once

#include <stdint.h>
#include "esp_err.h"

// Direct ST7796 SPI driver. No GUI framework — single full-screen test
// pattern for Stage 0 bring-up. Stage 1 adds digit-cell redraw on top of
// display_set_window()/display_push_pixels().

esp_err_t display_init(void);

// Fills the whole panel with a solid RGB565 color. Stage 0 bring-up check.
esp_err_t display_fill(uint16_t color_rgb565);

// Sets the active draw window (inclusive pixel coords) ahead of a pixel push.
esp_err_t display_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

// Pushes raw RGB565 pixel data into the window set by display_set_window().
esp_err_t display_push_pixels(const uint16_t *pixels, size_t count);
