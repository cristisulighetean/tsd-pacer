#include "display.h"
#include "app_config.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

static const char *TAG = "display";

// Panel resolution, per the board's 3.5" IPS panel.
#define PANEL_WIDTH  320
#define PANEL_HEIGHT 480

// Standard MIPI DCS commands (common across ST7796/ILI9341-family panels).
#define CMD_SWRESET 0x01
#define CMD_SLPOUT  0x11
#define CMD_COLMOD  0x3A
#define CMD_MADCTL  0x36
#define CMD_DISPON  0x29
#define CMD_CASET   0x2A
#define CMD_RASET   0x2B
#define CMD_RAMWR   0x2C

static spi_device_handle_t s_spi;

static esp_err_t send_cmd(uint8_t cmd, const uint8_t *data, size_t len)
{
    gpio_set_level(TSD_PIN_LCD_DC, 0); // command
    spi_transaction_t t = {.length = 8, .tx_buffer = &cmd};
    esp_err_t err = spi_device_polling_transmit(s_spi, &t);
    if (err != ESP_OK || len == 0) {
        return err;
    }
    gpio_set_level(TSD_PIN_LCD_DC, 1); // data
    spi_transaction_t td = {.length = len * 8, .tx_buffer = data};
    return spi_device_polling_transmit(s_spi, &td);
}

esp_err_t display_init(void)
{
    if (TSD_PIN_LCD_CS < 0 || TSD_PIN_LCD_DC < 0 || TSD_PIN_LCD_RST < 0) {
        ESP_LOGE(TAG, "LCD pins not set — confirm them from the board schematic "
                      "and fill in app_config.h before bring-up");
        return ESP_ERR_INVALID_STATE;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TSD_PIN_LCD_DC) | (1ULL << TSD_PIN_LCD_RST),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);
    if (TSD_PIN_LCD_BL >= 0) {
        gpio_config_t bl_conf = {.pin_bit_mask = 1ULL << TSD_PIN_LCD_BL, .mode = GPIO_MODE_OUTPUT};
        gpio_config(&bl_conf);
        gpio_set_level(TSD_PIN_LCD_BL, 1);
    }

    spi_bus_config_t bus_cfg = {
        .max_transfer_sz = PANEL_WIDTH * 16 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 40 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = TSD_PIN_LCD_CS,
        .queue_size = 4,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_cfg, &s_spi));

    gpio_set_level(TSD_PIN_LCD_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(TSD_PIN_LCD_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    send_cmd(CMD_SWRESET, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));
    send_cmd(CMD_SLPOUT, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));

    uint8_t colmod = 0x55; // 16 bpp
    send_cmd(CMD_COLMOD, &colmod, 1);

    // TODO: confirm MADCTL orientation/mirror bits on real hardware —
    // 0x00 is a placeholder, not verified against the physical mounting.
    uint8_t madctl = 0x00;
    send_cmd(CMD_MADCTL, &madctl, 1);

    send_cmd(CMD_DISPON, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(20));

    ESP_LOGI(TAG, "display init done");
    return ESP_OK;
}

esp_err_t display_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t col[4] = {x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF};
    uint8_t row[4] = {y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF};
    esp_err_t err = send_cmd(CMD_CASET, col, sizeof(col));
    if (err != ESP_OK) return err;
    err = send_cmd(CMD_RASET, row, sizeof(row));
    if (err != ESP_OK) return err;
    return send_cmd(CMD_RAMWR, NULL, 0);
}

esp_err_t display_push_pixels(const uint16_t *pixels, size_t count)
{
    gpio_set_level(TSD_PIN_LCD_DC, 1);
    spi_transaction_t t = {
        .length = count * 16,
        .tx_buffer = pixels,
    };
    return spi_device_polling_transmit(s_spi, &t);
}

esp_err_t display_fill(uint16_t color_rgb565)
{
    esp_err_t err = display_set_window(0, 0, PANEL_WIDTH - 1, PANEL_HEIGHT - 1);
    if (err != ESP_OK) return err;

    uint16_t line[PANEL_WIDTH];
    for (int i = 0; i < PANEL_WIDTH; i++) {
        line[i] = color_rgb565;
    }
    for (int y = 0; y < PANEL_HEIGHT; y++) {
        err = display_push_pixels(line, PANEL_WIDTH);
        if (err != ESP_OK) return err;
    }
    return ESP_OK;
}
