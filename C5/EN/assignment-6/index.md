---
title: "ESP-IDF and ESP32-C5 Workshop - Assignment 6"
date: 2024-09-30T00:00:00+01:00
layout: default
---

## Assignment 6: Low Power core

---

The ESP32-C5 features two cores: a high-power (HP) core for standard use (networking, complex computations) and a low-power (LP) RISC-V core designed to minimize energy consumption.

The LP core (often called the **ULP - Ultra Low Power core**) is designed to handle simple tasks while the main HP core is in deep sleep mode. This significantly reduces battery consumption, making it perfect for battery-powered projects. The LP core can collect data from sensors, control GPIOs, or increment a counter, waking up the HP core only when a threshold is met.

### 1. The Sleep Cycle

In this assignment, we will configure the HP core to go into Deep Sleep. The LP core will continue to run, perform a basic task, and eventually wake the HP core back up.

To use the LP core, we need to create a special sub-folder in our project called `main/ulp` and write the code that will run on the LP core there.

### 2. Configuration via menuconfig

Run:
```bash
idf.py menuconfig
```

Navigate to:
- **Component config ---> Ultra Low Power (ULP) Co-processor**
  - **Enable Ultra Low Power (ULP) Coprocessor**: Select `LP-core`
  - Leave other settings as default.

Press `Q` to quit and `Y` to save.

### 3. Writing the LP Core Code

Create a file named `main/ulp/main.c`. This code runs *only* on the LP core!

```c
#include <stdint.h>
#include "ulp_lp_core.h"
#include "ulp_lp_core_utils.h"

// Variables shared between HP and LP core
volatile uint32_t counter = 0;

int main(void)
{
    // Increment our counter
    counter++;

    // Wake the main CPU once the counter reaches 5
    if (counter >= 5) {
        ulp_lp_core_wakeup_main_processor();
        counter = 0;
    }

    // Go back to sleep until the LP timer wakes us again
    ulp_lp_core_halt();
    return 0;
}
```

### 4. Writing the HP Core Code (app_main)

In your `main/main.c`, initialize the LP core and then put the HP core to sleep:

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "ulp_lp_core.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

// This variable is mapped to the one in the LP core code!
extern uint32_t ulp_counter;

static const char *TAG = "lp_core";

void app_main(void)
{
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
```

### 5. CMake Configuration

The build system needs to know about the LP core code. Open `main/CMakeLists.txt` and add the following lines at the end:

```cmake
ulp_embed_binary(ulp_main "ulp/main.c")
```

### 6. Build, Flash, and Monitor

```bash
idf.py build flash monitor
```

You should see the HP core boot, start the LP core, and go to sleep. After 5 seconds, the LP core will wake the HP core back up, and you'll see the message: `Woken up by LP core!`

## Conclusion

Congratulations! You have completed the ESP32-C5 workshop. You've learned how to connect to Wi-Fi, perform OTA updates, use ESP-NOW for peer-to-peer communication, and utilize the LP core to save battery life. 

Thank you for participating!
