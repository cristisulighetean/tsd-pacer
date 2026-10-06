#include "display.h"

#include "esp_log.h"

static const char *TAG = "tsd_pacer";

void app_main(void)
{
    ESP_LOGI(TAG, "TSD Pacer boot");

    if (display_init() == ESP_OK) {
        display_fill(0x07E0); // green — Stage 0 bring-up check
    }
}
