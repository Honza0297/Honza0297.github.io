#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "ulp_lp_core.h"
#include "driver/gpio.h"

#define BUTTON_GPIO GPIO_NUM_0

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

extern uint32_t ulp_counter;

static const char *TAG = "lp_core";

static void configure_button_wakeup(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // external pull-down resistor, same as Assignment 4
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Wake up as soon as the button pulls GPIO0 high, even while deep asleep
    ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(1ULL << BUTTON_GPIO, ESP_GPIO_WAKEUP_GPIO_HIGH));
}

void app_main(void)
{
    // Delay to allow USB serial connection to attach before printing
    vTaskDelay(pdMS_TO_TICKS(1000));

    configure_button_wakeup();

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    if (cause == ESP_SLEEP_WAKEUP_ULP) {
        ESP_LOGI(TAG, "Woken up by the LP core! LP Counter: %lu", ulp_counter);
    } else if (cause == ESP_SLEEP_WAKEUP_GPIO) {
        ESP_LOGI(TAG, "Woken up by the button!");
    } else {
        ESP_LOGI(TAG, "Regular boot. Starting LP core...");

        // Load the LP core binary
        ESP_ERROR_CHECK(ulp_lp_core_load_binary(ulp_main_bin_start,
                                                (ulp_main_bin_end - ulp_main_bin_start)));

        // Configure the LP core to wake up every 1 second
        ulp_lp_core_cfg_t cfg = {
            .wakeup_source = ULP_LP_CORE_WAKEUP_SOURCE_LP_TIMER,
            .lp_timer_sleep_duration_us = 1000000,
        };
        ESP_ERROR_CHECK(ulp_lp_core_run(&cfg));
    }

    ESP_LOGI(TAG, "HP Core going to deep sleep. The LP core will wake us up after 5 counts, or press the button to wake up right away.");

    // Allow the LP core to wake the main processor too
    ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());

    // Go to deep sleep
    esp_deep_sleep_start();
}
