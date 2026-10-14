#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_wifi.h"
#include "esp_crt_bundle.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "led_strip.h"

#define BUTTON_GPIO       GPIO_NUM_0
#define LED_GPIO          27
#define FALLBACK_SSID     "ESP32"
#define FALLBACK_PASSWORD "ESP32"

static const char *TAG = "ota_example";
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

static void ota_task(void *pvParameter)
{
    while (1) {
        // Block here until button_task wakes us up - no auto-update on boot anymore!
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_LOGI(TAG, "Starting OTA update");
        led_set_color(20, 20, 20); // white: downloading

        esp_http_client_config_t config = {
            .url = "http://192.168.4.1/firmware.bin",
            // In production, serve this over HTTPS and check the server certificate:
            // .crt_bundle_attach = esp_crt_bundle_attach,
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
            ESP_LOGE(TAG, "Firmware upgrade failed");
            led_set_color(20, 0, 0); // red: failed
            vTaskDelay(pdMS_TO_TICKS(3000));
            led_set_color(0, 0, 20); // back to blue: ready, feel free to try again
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Running firmware v1!");

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

    // Prefer credentials stored in NVS by the Assignment 3 provisioning step.
    // If that never happened, connect to the fallback network instead.
    wifi_config_t wifi_config = { 0 };
    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &wifi_config));
    if (wifi_config.sta.ssid[0] == '\0') {
        ESP_LOGW(TAG, "No Wi-Fi credentials in NVS, using fallback SSID \"%s\"", FALLBACK_SSID);
        wifi_config_t fallback = {
            .sta = {
                .ssid = FALLBACK_SSID,
                .password = FALLBACK_PASSWORD,
                .threshold.authmode = WIFI_AUTH_WEP, // "ESP32" is 5 characters; WPA2 needs 8
            },
        };
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &fallback));
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Create the OTA task first so button_task has a valid handle to notify
    xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, &s_ota_task_handle);
    xTaskCreate(&button_task, "button_task", 2048, NULL, 5, NULL);
}
