#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_err.h"

/* RMT tick rate. A WS2812 bit lasts 1.25 us, so 10 MHz is about 10x finer. */
#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)
static const char *TAG = "workshop";

/* Set up RMT (it times the data line) and the one built-in LED. */
led_strip_handle_t configure_led(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = 27,
        .max_leds = 1,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, /* WS2812 wants G, then R, then B */
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false, /* true only if something flips the data line */
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_STRIP_RMT_RES_HZ,
        .mem_block_symbols = 0, /* 0 = let the driver pick the buffer size */
        .flags.with_dma = false, /* one LED is a few bytes; DMA is for long strips */
    };

    led_strip_handle_t led_strip;
    /* Create the driver. From here the handle is the LED. */
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "LED strip initialized successfully");
    return led_strip;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Hello ESP32-C5!");

    led_strip_handle_t led_strip = configure_led();

    /* set_pixel writes RAM; refresh sends that buffer to the LED. */
    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 20, 0, 0));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    ESP_LOGI(TAG, "LED set to Red");

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS); /* sleep 1 s instead of spinning */
    }
}
