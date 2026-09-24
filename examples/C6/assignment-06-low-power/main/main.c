#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "ulp_lp_core.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

extern uint32_t ulp_counter;

static const char *TAG = "lp_core";

void app_main(void)
{
    // Delay to allow USB serial connection to attach before printing
    vTaskDelay(pdMS_TO_TICKS(1000));

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    if (cause == ESP_SLEEP_WAKEUP_ULP) {
        ESP_LOGI(TAG, "Woken up by LP core! LP Counter: %lu", ulp_counter);
    } else {
        ESP_LOGI(TAG, "Regular boot. Starting LP core...");

        // Load the LP core binary
        ESP_ERROR_CHECK(ulp_lp_core_load_binary(ulp_main_bin_start,
                                                (ulp_main_bin_end - ulp_main_bin_start)));

        // Configure the LP core to wake up every 1 second
        ulp_lp_core_cfg_t cfg = {
            .wakeup_source = ULP_LP_CORE_WAKEUP_SOURCE_HP_TIMER,
            .hp_timer_sleep_duration_us = 1000000,
        };
        ESP_ERROR_CHECK(ulp_lp_core_run(&cfg));
    }

    ESP_LOGI(TAG, "HP Core going to deep sleep for 10 seconds, but ULP might wake us sooner...");
    
    // Allow ULP to wake the main processor
    ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());
    
    // Go to deep sleep
    esp_deep_sleep_start();
}
