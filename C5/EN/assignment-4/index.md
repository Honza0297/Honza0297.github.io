---
title: "ESP-IDF and ESP32-C5 Workshop - Assignment 4: OTA Updates"
date: 2024-06-30T00:00:00+01:00
layout: default
---

## Assignment 4: Over-The-Air (OTA) Updates

---

> **Note:** This is an optional chapter. If time is running short, you may skip directly to [Assignment 5: ESP-NOW Chat Room](../assignment-5/).

In a real-world deployment, you won't have physical access to a USB port to flash new firmware. Over-The-Air (OTA) updates allow you to download and install new firmware over a network connection like Wi-Fi.

> **Keep reusing the project.** Continue in the same project you provisioned in Assignment 3 — OTA needs a device that can already connect to your Wi-Fi, and we rely on the credentials you stored in NVS back then. We'll replace `main.c` again, but keep `idf_component.yml` as-is: it already pulls in `led_strip` from Assignment 2, and we're about to need it again.

### 1. Partition Tables

To support OTA, the flash memory of the ESP32-C5 must be divided into specific regions called partitions. A typical OTA partition table includes:
- **nvs**: Non-Volatile Storage for Wi-Fi credentials and calibration data.
- **otadata**: Stores information about which app partition is currently active.
- **phy_init**: Factory-calibrated radio data.
- **ota_0**: The first app slot - this is what gets flashed initially.
- **ota_1**: The second app slot, where new firmware gets downloaded to.

When an OTA update occurs, the device downloads the new firmware into the inactive partition (whichever of `ota_0`/`ota_1` isn't currently running). If the download is successful, it updates the `otadata` partition to point there and reboots.

We define this layout in a file named `partitions.csv` in our project root:

```text
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     ,        0x4000,
otadata,  data, ota,     ,        0x2000,
phy_init, data, phy,     ,        0x1000,
ota_0,    app,  ota_0,   ,        1500K,
ota_1,    app,  ota_1,   ,        1500K,
```

1500K per slot gives the OTA/TLS/Wi-Fi/LED code in this assignment comfortable headroom (the actual binary is roughly 1.1 MB) while still leaving two full slots on a 4 MB flash chip.

> **Tip:** If you'd rather not edit CSV files by hand, the ESP-IDF VS Code extension ships GUI editors for both the partition table and NVS contents.
>
> ![ESP-IDF Partition Table Editor](assets/nvs-1.webp)
>
> ![ESP-IDF NVS Partition Editor](assets/nvs-2.webp)

### 2. Configuration via menuconfig

To tell ESP-IDF to use our custom partition table, run:

```bash
idf.py menuconfig
```

Navigate to:
- **Partition Table**
  - **Partition Table**: Select `Custom partition table CSV`
  - **Custom partition CSV file**: Type `partitions.csv`
- **Serial Flasher Config**
  - **Flash size**: Select `4 MB` — two 1500K app slots plus `nvs`/`otadata`/`phy_init` no longer fit in the default 2 MB assumption.

Press `Q` to quit and `Y` to save.

> If you'd rather not click through menuconfig for this, drop both settings into a `sdkconfig.defaults` file in your project root instead:
> ```text
> CONFIG_PARTITION_TABLE_CUSTOM=y
> CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"
> CONFIG_PARTITION_TABLE_FILENAME="partitions.csv"
> CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
> ```

### 3. Connecting a Button

Instead of updating itself the moment it boots, our board will wait for you to press a physical button before it starts downloading anything. This is both safer (no surprise downloads mid-demo) and a good excuse to learn basic GPIO input — and we'll keep reusing this exact button in Assignments 5 and 6, so it's worth wiring up carefully now.

Any simple 2-pin or 4-pin tactile push button works. On a 4-pin button, the two pins on each short side are internally connected to each other:

![Tactile button pinout - the two pins on each side are internally connected](assets/button.webp)

Wire it like this:

- One leg of the button -> **3V3**
- The other leg of the button -> **GPIO0**
- A pull-down resistor (e.g. 10 kΩ) between **GPIO0** and **GND**

With the pull-down resistor in place, `GPIO0` reads **LOW** while the button is released, and **HIGH** while it's pressed (because the button then connects it straight to 3V3).

> `GPIO0` is also a boot-mode strapping pin on many ESP32 chips, but that only matters for the brief instant right at power-up/reset. Pressing the button during normal operation (which is all we ever do here) has no effect on that.

### 4. Writing the Code

As before, let's build this up instead of pasting it all at once.

#### 4.1 Skeleton

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_https_ota.h"
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "esp_crt_bundle.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "led_strip.h"

#define BUTTON_GPIO GPIO_NUM_0
#define LED_GPIO    27

// (A) LED helpers - reused from Assignment 2
// (B) Button GPIO setup
// (C) Wi-Fi event handler - also updates the LED
// (D) A task that polls the button and wakes up the OTA task
// (E) The OTA task itself - now waits for a notification instead of running immediately
// (F) app_main wires everything together
```

#### 4.2 Part (A)-(B): LED and button setup

```c
static led_strip_handle_t led_strip;
static TaskHandle_t s_ota_task_handle = NULL;

static void led_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    led_strip_set_pixel(led_strip, 0, r, g, b);
    led_strip_refresh(led_strip);
}

static void configure_led(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 0,
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
}

static void configure_button(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // we use an external pull-down resistor
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}
```

We'll use the LED as a status light with a simple color code we'll build up through this assignment:
- **Yellow** - connecting / reconnecting to Wi-Fi
- **Blue** - ready, waiting for the button
- **White** - downloading the update
- **Green** - update successful, about to reboot
- **Red** - update failed, back to blue after a few seconds

#### 4.3 Part (C): the Wi-Fi event handler

```c
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
        led_set_color(20, 20, 0); // yellow: (re)connecting
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Ready. Press the button to start the OTA update.");
        led_set_color(0, 0, 20); // blue: ready, waiting for the button
    }
}
```

This is the same Wi-Fi reconnect logic you've already seen, just extended to flip the LED to yellow or blue as the connection state changes.

#### 4.4 Part (D): the button task

```c
// Polls the button and wakes ota_task up on a press. A dedicated task keeps
// the debounce logic simple and out of the way of everything else.
static void button_task(void *pvParameter)
{
    bool was_pressed = false;
    while (1) {
        bool pressed = gpio_get_level(BUTTON_GPIO) == 1;
        if (pressed && !was_pressed) {
            vTaskDelay(pdMS_TO_TICKS(50)); // simple debounce
            if (gpio_get_level(BUTTON_GPIO) == 1) {
                ESP_LOGI(TAG, "Button pressed - triggering OTA update");
                xTaskNotifyGive(s_ota_task_handle);
            }
        }
        was_pressed = pressed;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
```

Rather than wiring up a GPIO interrupt (easy to get wrong when you're just starting out), this task simply polls the pin every 20 ms. When it sees the signal go from low to high, it waits 50 ms and double-checks the pin is still high — a simple but effective way to ignore the tiny electrical bounces a mechanical switch produces — then wakes `ota_task` up with `xTaskNotifyGive`.

#### 4.5 Part (E): the OTA task

```c
static void ota_task(void *pvParameter)
{
    while (1) {
        // Block here until button_task wakes us up - no auto-update on boot anymore!
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_LOGI(TAG, "Starting OTA update");
        led_set_color(20, 20, 20); // white: downloading

        esp_http_client_config_t config = {
            .url = "https://example.com/firmware.bin",
            // In production, always provide a certificate!
            // .cert_pem = (char *)server_cert_pem_start,
            .crt_bundle_attach = esp_crt_bundle_attach,
        };
        esp_https_ota_config_t ota_config = {
            .http_config = &config,
        };

        ESP_LOGI(TAG, "Attempting to download update from %s", config.url);
        esp_err_t ret = esp_https_ota(&ota_config);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "OTA succeeded, rebooting...");
            led_set_color(0, 20, 0); // green: success
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_restart();
        } else {
            ESP_LOGE(TAG, "Firmware upgrade failed (expected if example.com is used)");
            led_set_color(20, 0, 0); // red: failed
            vTaskDelay(pdMS_TO_TICKS(3000));
            led_set_color(0, 0, 20); // back to blue: ready, feel free to try again
        }
    }
}
```

`ulTaskNotifyTake(pdTRUE, portMAX_DELAY)` is the key change from before: the task just sleeps until `button_task` notifies it, instead of starting the download automatically after a fixed delay. Everything else — the `esp_https_ota` call itself — is unchanged from how OTA always worked.

#### 4.6 Part (F): `app_main`

```c
void app_main(void)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    configure_led();
    configure_button();
    led_set_color(20, 20, 0); // yellow: connecting

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // Relies on credentials already stored in NVS by the Assignment 3 provisioning step
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Create the OTA task first so button_task has a valid handle to notify
    xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, &s_ota_task_handle);
    xTaskCreate(&button_task, "button_task", 2048, NULL, 5, NULL);
}
```

#### 4.7 Complete code

The full file lives at [`examples/C5/assignment-04-ota`](../../../examples/C5/assignment-04-ota/), alongside the matching `partitions.csv` and `idf_component.yml` — grab it from there if you'd rather not type everything out, or use it to check your work.

### 5. Build, Flash, and Monitor

```bash
idf.py build flash monitor
```

Watch the LED: it should turn **yellow** while connecting, then **blue** once Wi-Fi comes up. Press the button — the LED turns **white** while it attempts the download, then either **green** (and the board reboots) or **red** for a few seconds before returning to blue, since `https://example.com/firmware.bin` isn't a real firmware file. See [step 7](#7-building-and-hosting-a-real-update-binary-self-paced-readers) below for how to point `config.url` at a `.bin` you actually built, to see the full happy path.

### 6. Rollback (Bonus)

ESP-IDF also supports App Rollback. If a newly flashed firmware panics or fails to connect to the server within a certain time frame, the bootloader will mark it as invalid and boot back into the previous, working firmware partition.

### 7. Building and Hosting a Real Update Binary (Self-Paced Readers)

> **Running this workshop with a tutor?** They've likely already set up a server and will give you a URL to use — you can skip this section and just update `config.url` with whatever you're given.

If you're going through this on your own and want to see a real update happen (not just a failed download against `example.com`), here's the shortest path:

**1. Make a visible change.** So you can actually tell the update worked, change something obvious before rebuilding — for example, bump a version string:

```c
ESP_LOGI(TAG, "Running firmware v2!");
```

**2. Build it.**

```bash
idf.py build
```

The binary you need is `build/<your-project-name>.bin` (the same file `idf.py flash` normally uses).

**3. Host it somewhere with a real HTTPS certificate.** The code in this assignment uses `.crt_bundle_attach = esp_crt_bundle_attach`, which checks the server's certificate against a bundle of well-known public Certificate Authorities. This means the easiest option is to put the `.bin` somewhere that already has valid HTTPS, without running any server yourself — for example:
- Push it to a (public or private-but-accessible) GitHub repo and use the `raw.githubusercontent.com` link to the file.
- Upload it to any cloud storage bucket that serves files over HTTPS (S3, Google Cloud Storage, etc.) with public read access.

Point `config.url` at that link, rebuild and reflash with your *original* (v1) firmware, and press the button. You should see the LED go white, then green, and the board should reboot running the new binary — check the log line you changed in step 1 to confirm.

> **Want to do it fully offline instead?** You can run a local HTTPS server (e.g. `python -m http.server` wrapped with a self-signed TLS certificate) but then `esp_crt_bundle_attach` won't trust it — a self-signed cert isn't in the public CA bundle. You'd need to switch `esp_https_ota_config_t` from `.crt_bundle_attach` to `.cert_pem`, embedding your self-signed certificate into the firmware as the one it should trust. This works, but it's more moving parts than most self-paced readers need just to see OTA work once.

## Next step

Let's dive into an entirely different wireless technology: ESP-NOW! The button and LED you just wired up are coming with us.

[Assignment 5: ESP-NOW Chat Room](../assignment-5/)
