---
title: "ESP-IDF and ESP32-C5 Workshop - Assignment 5: ESP-NOW Chat Room"
date: 2024-06-30T00:00:00+01:00
layout: default
---

## Assignment 5: ESP-NOW Chat Room

---

While Wi-Fi is great for connecting to the Internet, it requires an Access Point (router) and has overhead due to the connection process. What if you just want multiple ESP32 boards to talk directly to each other?

**ESP-NOW** is a connectionless, low-overhead wireless communication protocol defined by Espressif. It allows multiple devices to communicate with each other peer-to-peer (P2P) without a router.

In this assignment, we will turn our workshop room into a giant ESP-NOW Chat Room! Every participant's board will broadcast commands, and when another board receives a specific command, it will change its LED color.

### 1. Initializing ESP-NOW

ESP-NOW requires the Wi-Fi peripheral to be turned on in Station or SoftAP mode, even though we aren't connecting to a router.

```c
ESP_ERROR_CHECK(esp_netif_init());
ESP_ERROR_CHECK(esp_event_loop_create_default());
wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
ESP_ERROR_CHECK(esp_wifi_init(&cfg));
ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
ESP_ERROR_CHECK(esp_wifi_start());
```

Then we initialize ESP-NOW itself:

```c
ESP_ERROR_CHECK(esp_now_init());
```

### 2. Adding Peers

To broadcast to everyone in the room, we add a peer with the broadcast MAC address `FF:FF:FF:FF:FF:FF`.

```c
esp_now_peer_info_t peerInfo = {};
memset(&peerInfo, 0, sizeof(peerInfo));
// Broadcast MAC address
for (int i = 0; i < 6; i++) {
    peerInfo.peer_addr[i] = 0xFF;
}
peerInfo.channel = 0; // Use current channel
peerInfo.encrypt = false; // No encryption for broadcast

ESP_ERROR_CHECK(esp_now_add_peer(&peerInfo));
```

### 3. Receiving Commands

We register a callback function that executes whenever an ESP-NOW packet is received. This function will parse the incoming string and change our LED color!

```c
void on_data_recv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    // data might not be null terminated, so let's copy it safely
    char msg[32] = {0};
    int len = data_len < 31 ? data_len : 31;
    memcpy(msg, data, len);

    ESP_LOGI("ESP-NOW", "Received: %s", msg);

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

// In app_main:
ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_recv));
```

### 4. Sending Commands

Let's periodically broadcast a command to the room! Alternatively, you could hook this up to a physical button on the board. For now, we will just send it in a loop.

```c
uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
char *msg = "COLOR GREEN";

while(1) {
    esp_err_t result = esp_now_send(broadcast_mac, (uint8_t *)msg, strlen(msg));
    if (result == ESP_OK) {
        ESP_LOGI("ESP-NOW", "Broadcast sent!");
    } else {
        ESP_LOGE("ESP-NOW", "Send error");
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
}
```

### 5. Build, Flash, and Monitor

Make sure you have the `led_strip` component added:
```bash
idf.py add-dependency "espressif/led_strip^3.0.3"
```

Then build and flash:
```bash
idf.py build flash monitor
```

Work with the person next to you: Have one person broadcast "COLOR RED" and the other broadcast "COLOR GREEN". See your LEDs sync up!

## Next step

Let's look at one final feature of the ESP32-C5: The Low-Power core!

[Assignment 6: LP-core](../assignment-6/)
