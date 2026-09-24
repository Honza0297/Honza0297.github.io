#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "led_strip.h"

static const char *TAG = "esp_now_chat";
static led_strip_handle_t led_strip;
uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void on_data_recv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len)
{
    char msg[32] = {0};
    int len = data_len < 31 ? data_len : 31;
    memcpy(msg, data, len);

    ESP_LOGI(TAG, "Received: %s", msg);

    if (strcmp(msg, "LED ON") == 0) {
        led_strip_set_pixel(led_strip, 0, 20, 20, 20); // White
        led_strip_refresh(led_strip);
    } else if (strcmp(msg, "LED OFF") == 0) {
        led_strip_clear(led_strip);
    } else if (strcmp(msg, "COLOR RED") == 0) {
        led_strip_set_pixel(led_strip, 0, 20, 0, 0); // Red
        led_strip_refresh(led_strip);
    } else if (strcmp(msg, "COLOR GREEN") == 0) {
        led_strip_set_pixel(led_strip, 0, 0, 20, 0); // Green
        led_strip_refresh(led_strip);
    }
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize LED
    led_strip_config_t strip_config = {
        .strip_gpio_num = 8,
        .max_leds = 1,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .mem_block_symbols = 0,
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);

    // Initialize Wi-Fi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Initialize ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_recv));

    // Add Broadcast Peer
    esp_now_peer_info_t peerInfo = {};
    memset(&peerInfo, 0, sizeof(peerInfo));
    for (int i = 0; i < 6; i++) {
        peerInfo.peer_addr[i] = 0xFF;
    }
    peerInfo.channel = 0; 
    peerInfo.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peerInfo));

    ESP_LOGI(TAG, "ESP-NOW Chat Room Started!");

    // Periodic broadcast
    char *msg = "COLOR GREEN"; // Change this to broadcast different commands
    while(1) {
        esp_now_send(broadcast_mac, (uint8_t *)msg, strlen(msg));
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
