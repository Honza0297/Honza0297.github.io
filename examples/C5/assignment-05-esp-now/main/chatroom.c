#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_now.h"
#include "led_strip.h"
#include "chatroom.h"

#define LED_GPIO 27

/* Add a row to teach the room a new color word. Other boards only see the numbers. */
#define COMMAND_RED    20, 0, 0
#define COMMAND_GREEN   0, 20, 0
#define COMMAND_BLUE    0, 0, 20
#define COMMAND_WHITE  20, 20, 20
#define COMMAND_ON     20, 20, 20
#define COMMAND_OFF     0, 0, 0

typedef struct {
    const char *name;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} command_t;

static const command_t s_commands[] = {
    { "red",   COMMAND_RED },
    { "green", COMMAND_GREEN },
    { "blue",  COMMAND_BLUE },
    { "white", COMMAND_WHITE },
    { "on",    COMMAND_ON },
    { "off",   COMMAND_OFF },
};

/* One ESP-NOW peer slot is the broadcast address, so 19 names can be remembered. */
#define CHATROOM_MAX_CONTACTS 19

typedef struct {
    char name[CHATROOM_NAME_LEN + 1];
    uint8_t mac[6];
    bool used;
    bool peer_added;
} contact_t;

static const char *TAG = "chatroom";
static char s_node_name[CHATROOM_NAME_LEN + 1];
static led_strip_handle_t s_led;
static contact_t s_contacts[CHATROOM_MAX_CONTACTS];
static SemaphoreHandle_t s_book_mutex;
static QueueHandle_t s_rx_queue;

typedef struct {
    uint8_t mac[6];
    uint16_t len;
    uint8_t data[ESP_NOW_MAX_DATA_LEN];
} rx_packet_t;

static bool bytes_are_ascii(const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (data[i] < 0x20 || data[i] > 0x7E) {
            return false;
        }
    }
    return true;
}

static void led_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    led_strip_set_pixel(s_led, 0, r, g, b);
    led_strip_refresh(s_led);
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
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led));
    led_strip_clear(s_led);
}

static const command_t *find_command(const char *text)
{
    for (size_t i = 0; i < sizeof(s_commands) / sizeof(s_commands[0]); i++) {
        if (strcmp(text, s_commands[i].name) == 0) {
            return &s_commands[i];
        }
    }
    return NULL;
}

/* "20,0,0" is a color. Anything else, including extra characters, is chat text. */
static bool parse_rgb(const char *s, int *r, int *g, int *b)
{
    int values[3];
    for (int i = 0; i < 3; i++) {
        if (*s < '0' || *s > '9') {
            return false;
        }
        int value = 0;
        int digits = 0;
        while (*s >= '0' && *s <= '9') {
            if (digits >= 3) {
                return false;
            }
            value = value * 10 + (*s - '0');
            if (value > 255) {
                return false;
            }
            digits++;
            s++;
        }
        if (i < 2) {
            if (*s != ',') {
                return false;
            }
            s++;
        }
        values[i] = value;
    }
    if (*s != '\0') {
        return false;
    }
    *r = values[0];
    *g = values[1];
    *b = values[2];
    return true;
}

static bool lookup_contact(const char *name, uint8_t mac[6])
{
    bool found = false;
    xSemaphoreTake(s_book_mutex, portMAX_DELAY);
    for (int i = 0; i < CHATROOM_MAX_CONTACTS; i++) {
        if (s_contacts[i].used && strcmp(s_contacts[i].name, name) == 0) {
            memcpy(mac, s_contacts[i].mac, 6);
            found = true;
            break;
        }
    }
    xSemaphoreGive(s_book_mutex);
    return found;
}

static void remember_contact(const char *name, const uint8_t mac[6])
{
    bool add_peer = false;
    bool full = false;
    uint8_t mac_copy[6];

    xSemaphoreTake(s_book_mutex, portMAX_DELAY);
    contact_t *slot = NULL;
    for (int i = 0; i < CHATROOM_MAX_CONTACTS; i++) {
        if (s_contacts[i].used && strcmp(s_contacts[i].name, name) == 0) {
            slot = &s_contacts[i];
            break;
        }
    }
    if (slot == NULL) {
        for (int i = 0; i < CHATROOM_MAX_CONTACTS; i++) {
            if (!s_contacts[i].used) {
                slot = &s_contacts[i];
                memcpy(slot->name, name, CHATROOM_NAME_LEN + 1);
                slot->used = true;
                slot->peer_added = false;
                break;
            }
        }
    }
    if (slot == NULL) {
        full = true;
    } else if (!slot->peer_added || memcmp(slot->mac, mac, 6) != 0) {
        memcpy(slot->mac, mac, 6);
        slot->peer_added = false;
        memcpy(mac_copy, mac, 6);
        add_peer = true;
    }
    xSemaphoreGive(s_book_mutex);

    if (full) {
        ESP_LOGW(TAG, "Contact book is full, not saving %.8s", name);
        return;
    }
    if (!add_peer) {
        return;
    }

    if (espnow_add_peer(mac_copy) != ESP_OK) {
        ESP_LOGW(TAG, "Could not add an ESP-NOW peer for %.8s", name);
        return;
    }

    xSemaphoreTake(s_book_mutex, portMAX_DELAY);
    if (slot->used && strcmp(slot->name, name) == 0 && memcmp(slot->mac, mac_copy, 6) == 0) {
        slot->peer_added = true;
    }
    xSemaphoreGive(s_book_mutex);
    ESP_LOGI(TAG, "Saved contact %.8s (%02x:%02x:%02x:%02x:%02x:%02x)",
             name, mac_copy[0], mac_copy[1], mac_copy[2], mac_copy[3], mac_copy[4], mac_copy[5]);
}

static bool send_payload(const uint8_t *dest_mac, const char *text)
{
    uint8_t packet[ESP_NOW_MAX_DATA_LEN];
    const size_t prefix = CHATROOM_NAME_LEN + 1; /* "name:" */
    size_t text_len = strlen(text);
    size_t max_text = ESP_NOW_MAX_DATA_LEN - prefix;

    memcpy(packet, s_node_name, CHATROOM_NAME_LEN);
    packet[CHATROOM_NAME_LEN] = ':';
    if (text_len > max_text) {
        ESP_LOGW(TAG, "Trimming the message to %u bytes", (unsigned)ESP_NOW_MAX_DATA_LEN);
        text_len = max_text;
    }
    memcpy(packet + prefix, text, text_len);

    esp_err_t err = dest_mac == NULL
        ? espnow_broadcast(packet, prefix + text_len)
        : espnow_send(dest_mac, packet, prefix + text_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Send failed: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

static void handle_line(char *line)
{
    if (line[0] == '\0') {
        return;
    }
    if (!bytes_are_ascii((const uint8_t *)line, strlen(line))) {
        ESP_LOGW(TAG, "Only ASCII characters are allowed");
        return;
    }

    const uint8_t *dest_mac = NULL;
    uint8_t mac[6];
    const char *body = line;

    if (strlen(line) >= CHATROOM_NAME_LEN + 1 && line[CHATROOM_NAME_LEN] == ':') {
        char name[CHATROOM_NAME_LEN + 1];
        memcpy(name, line, CHATROOM_NAME_LEN);
        name[CHATROOM_NAME_LEN] = '\0';
        if (!lookup_contact(name, mac)) {
            ESP_LOGW(TAG, "No contact named %s yet. Wait until that board sends something.", name);
            return;
        }
        dest_mac = mac;
        body = line + CHATROOM_NAME_LEN + 1;
        if (body[0] == '\0') {
            return;
        }
    }

    const command_t *command = find_command(body);
    char encoded[16];
    const char *text = body;
    if (command != NULL) {
        snprintf(encoded, sizeof(encoded), "%u,%u,%u", command->r, command->g, command->b);
        text = encoded;
    }

    if (!send_payload(dest_mac, text)) {
        return;
    }
    if (dest_mac == NULL) {
        ESP_LOGI(TAG, "Sent: %s", body);
    } else {
        ESP_LOGI(TAG, "Sent to %.8s: %s", line, body);
    }
}

static void deliver_packet(const uint8_t src_mac[6], const uint8_t *data, int len)
{
    if (len < CHATROOM_NAME_LEN + 1 || data[CHATROOM_NAME_LEN] != ':') {
        return;
    }
    if (!bytes_are_ascii(data, (size_t)len)) {
        ESP_LOGW(TAG, "Ignored a packet that was not ASCII");
        return;
    }

    char sender[CHATROOM_NAME_LEN + 1];
    memcpy(sender, data, CHATROOM_NAME_LEN);
    sender[CHATROOM_NAME_LEN] = '\0';

    if (memcmp(sender, s_node_name, CHATROOM_NAME_LEN) == 0) {
        ESP_LOGW(TAG, "Another board is using your name");
    } else {
        remember_contact(sender, src_mac);
    }

    char body[ESP_NOW_MAX_DATA_LEN + 1];
    int body_len = len - (CHATROOM_NAME_LEN + 1);
    memcpy(body, data + CHATROOM_NAME_LEN + 1, (size_t)body_len);
    body[body_len] = '\0';
    if (body[0] == '\0') {
        return;
    }

    int r, g, b;
    if (parse_rgb(body, &r, &g, &b)) {
        led_set_color((uint8_t)r, (uint8_t)g, (uint8_t)b);
        ESP_LOGI(TAG, "User %s set your LED to %d,%d,%d", sender, r, g, b);
    } else {
        ESP_LOGI(TAG, "User %s: %s", sender, body);
    }
}

static void rx_task(void *arg)
{
    rx_packet_t packet;
    while (1) {
        if (xQueueReceive(s_rx_queue, &packet, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        deliver_packet(packet.mac, packet.data, packet.len);
    }
}

static void input_task(void *arg)
{
    static char line[512];
    size_t used = 0;

    while (1) {
        int ch = getchar();
        if (ch == EOF) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }
        if (ch == '\r' || ch == '\n') {
            putchar('\n');
            line[used] = '\0';
            used = 0;
            handle_line(line);
            continue;
        }
        if (ch == '\b' || ch == 0x7F) {
            if (used > 0) {
                used--;
                printf("\b \b");
            }
            continue;
        }
        if (ch < 0x20 || used + 1 >= sizeof(line)) {
            continue;
        }
        line[used++] = (char)ch;
        putchar(ch);
    }
}

void chatroom_on_recv(const uint8_t src_mac[6], const uint8_t *data, int len)
{
    if (s_rx_queue == NULL || src_mac == NULL || data == NULL) {
        return;
    }
    if (len <= 0 || len > ESP_NOW_MAX_DATA_LEN) {
        return;
    }

    rx_packet_t packet = { 0 };
    memcpy(packet.mac, src_mac, 6);
    packet.len = (uint16_t)len;
    memcpy(packet.data, data, (size_t)len);
    if (xQueueSend(s_rx_queue, &packet, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Dropping a packet; the chat task is busy");
    }
}

void chatroom_start(const char *node_name)
{
    if (node_name == NULL || strlen(node_name) != CHATROOM_NAME_LEN ||
        !bytes_are_ascii((const uint8_t *)node_name, CHATROOM_NAME_LEN)) {
        ESP_LOGE(TAG, "Node name must be 8 ASCII characters");
        abort();
    }
    memcpy(s_node_name, node_name, CHATROOM_NAME_LEN + 1);

    configure_led();
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    s_book_mutex = xSemaphoreCreateMutex();
    s_rx_queue = xQueueCreate(8, sizeof(rx_packet_t));
    ESP_ERROR_CHECK(s_book_mutex != NULL && s_rx_queue != NULL ? ESP_OK : ESP_ERR_NO_MEM);

    xTaskCreate(rx_task, "chat_rx", 4096, NULL, 5, NULL);
    xTaskCreate(input_task, "chat_in", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "This board is \"%s\".", s_node_name);
    ESP_LOGI(TAG, "Broadcast a message, or: red, green, blue, white, on, off.");
    ESP_LOGI(TAG, "Direct message: <8-character name>:<message or command>.");
}
