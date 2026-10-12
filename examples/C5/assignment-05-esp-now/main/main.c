#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "led_strip.h"

#define BUTTON_GPIO GPIO_NUM_0
#define LED_GPIO    27

static const char *TAG = "esp_now_chat";
static led_strip_handle_t led_strip;
static uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Idle color: dim blue, meaning "powered up and listening"
#define LED_IDLE_R 0
#define LED_IDLE_G 0
#define LED_IDLE_B 5

static void led_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    led_strip_set_pixel(led_strip, 0, r, g, b);
    led_strip_refresh(led_strip);
}

// Briefly flashes a color, then returns to the idle "listening" blue
static void led_flash(uint8_t r, uint8_t g, uint8_t b, int duration_ms)
{
    led_set_color(r, g, b);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    led_set_color(LED_IDLE_R, LED_IDLE_G, LED_IDLE_B);
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
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
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

static void on_data_recv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len)
{
    char msg[32] = {0};
    int len = data_len < 31 ? data_len : 31;
    memcpy(msg, data, len);

    ESP_LOGI(TAG, "Received: %s", msg);

    // Received commands set the LED and keep it there (until the next command)
    if (strcmp(msg, "LED ON") == 0) {
        led_set_color(20, 20, 20); // white
    } else if (strcmp(msg, "LED OFF") == 0) {
        led_set_color(LED_IDLE_R, LED_IDLE_G, LED_IDLE_B);
    } else if (strcmp(msg, "COLOR RED") == 0) {
        led_set_color(20, 0, 0);
    } else if (strcmp(msg, "COLOR GREEN") == 0) {
        led_set_color(0, 20, 0);
    }
}

// Cycle through a handful of commands - one per button press
static const char *commands[] = {"COLOR RED", "COLOR GREEN", "LED ON", "LED OFF"};
#define NUM_COMMANDS (sizeof(commands) / sizeof(commands[0]))

static void send_next_command(void)
{
    static int next = 0;
    const char *msg = commands[next];
    next = (next + 1) % NUM_COMMANDS;

    ESP_LOGI(TAG, "Broadcasting: %s", msg);
    esp_err_t result = esp_now_send(broadcast_mac, (const uint8_t *)msg, strlen(msg));
    if (result == ESP_OK) {
        led_flash(20, 20, 20, 150); // white flash: sent
    } else {
        ESP_LOGE(TAG, "Send error");
        led_flash(20, 0, 0, 300); // red flash: send failed
    }
}

// Polls the button; one press broadcasts the next command in the list
static void button_task(void *pvParameter)
{
    bool was_pressed = false;
    while (1) {
        bool pressed = gpio_get_level(BUTTON_GPIO) == 1;
        if (pressed && !was_pressed) {
            vTaskDelay(pdMS_TO_TICKS(50)); // simple debounce
            if (gpio_get_level(BUTTON_GPIO) == 1) {
                send_next_command();
            }
        }
        was_pressed = pressed;
        vTaskDelay(pdMS_TO_TICKS(20));
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

    configure_led();
    configure_button();
    led_set_color(LED_IDLE_R, LED_IDLE_G, LED_IDLE_B); // dim blue: listening

    // Initialize Wi-Fi (ESP-NOW needs the radio on, but no AP connection)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Initialize ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_recv));

    // Add the broadcast peer so esp_now_send() can reach everyone in the room
    esp_now_peer_info_t peer_info = {};
    memset(&peer_info, 0, sizeof(peer_info));
    memcpy(peer_info.peer_addr, broadcast_mac, 6);
    peer_info.channel = 0; // use the current channel
    peer_info.encrypt = false; // broadcast traffic cannot be encrypted
    ESP_ERROR_CHECK(esp_now_add_peer(&peer_info));

    ESP_LOGI(TAG, "ESP-NOW Chat Room started! Press the button to broadcast a command.");

    xTaskCreate(&button_task, "button_task", 2048, NULL, 5, NULL);
}
