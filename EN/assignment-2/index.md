---
title: "ESP-IDF and ESP32-C6 Workshop - Assignment 2: Basics"
date: 2024-06-30T00:00:00+01:00
layout: default
---

## Assignment 2: Basics: Hello World & LED

---

In this assignment, we will create our first project, verify that we can build and flash it to the ESP32-C6 board, and learn how to use components via the command line interface (CLI).

### 1. Creating a new project

Instead of using a GUI wizard, we will use the ESP-IDF CLI. Open your ESP-IDF Terminal and navigate to a directory where you want to store your project.

```bash
idf.py create-project my_first_project
cd my_first_project
```

This creates a basic directory structure with a `CMakeLists.txt` and a `main` folder containing `main.c`.

### 2. Setting the target

Before building, we must tell ESP-IDF which chip we are compiling for. Run the following command:

```bash
idf.py set-target esp32c6
```

This generates a `sdkconfig` file with the default configuration for the ESP32-C6.

### 3. Adding a Component

We want to control the built-in RGB LED (WS2812) on our DevKit. This LED is connected to GPIO 27.
To control it easily, we will add the `led_strip` component from the Espressif Component Registry.

Run the following command in your project directory:

```bash
idf.py add-dependency "espressif/led_strip^3.0.3"
```

This creates a `main/idf_component.yml` file. When we build the project, ESP-IDF will automatically download the required component.

### 4. Writing the Code

Open `main/main.c` in VS Code and replace its contents with the following code. This code initializes the LED and sets it to red.

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_err.h"

#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)
static const char *TAG = "workshop";

led_strip_handle_t configure_led(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = 8,
        .max_leds = 1,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_STRIP_RMT_RES_HZ,
        .mem_block_symbols = 0,
        .flags.with_dma = false,
    };

    led_strip_handle_t led_strip;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "LED strip initialized successfully");
    return led_strip;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Hello ESP32-C6!");
    
    led_strip_handle_t led_strip = configure_led();
    
    // Set color to Red
    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 20, 0, 0));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    ESP_LOGI(TAG, "LED set to Red");
    
    while(1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
```

### 5. Building, Flashing, and Monitoring

Now we are ready to build the firmware, flash it to the board, and monitor the serial output. You can do all three in a single command!

Connect your ESP32-C6 to your computer using the USB Type-C port labeled "UART". Then run:

```bash
idf.py build flash monitor
```

* `build`: Compiles the code.
* `flash`: Uploads the binary to the board. ESP-IDF usually auto-detects the serial port.
* `monitor`: Opens a serial terminal to view the output.

> **Tip:** To exit the monitor, press `Ctrl + ]`.

If everything is correct, the built-in LED should light up red, and you will see "Hello ESP32-C6!" in your terminal.

## Next step

Now that our hardware is working, let's explore one of the most important features of the C6: Wi-Fi!

[Assignment 3: Wi-Fi & Provisioning](../assignment-3/)
