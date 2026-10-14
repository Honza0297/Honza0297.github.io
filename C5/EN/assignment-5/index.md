---
title: "ESP-IDF and ESP32-C5 Workshop - Assignment 5: ESP-NOW Chat Room"
date: 2024-06-30T00:00:00+01:00
layout: default
---

## Assignment 5: ESP-NOW Chat Room

---

While Wi-Fi is great for connecting to the Internet, it requires an Access Point (router) and has overhead due to the connection process. What if you just want multiple ESP32 boards to talk directly to each other?

### What is ESP-NOW, actually?

**ESP-NOW** is a connectionless, low-overhead wireless protocol that Espressif built on top of the Wi-Fi radio. A few things worth knowing as a tinkerer, before we touch any code:

- **No router, no handshake.** Devices don't "join a network" the way they do with normal Wi-Fi — there's no association, no DHCP, no IP address. A board just radios out a small packet, and anyone listening on the same Wi-Fi channel can pick it up. This is also why it's so fast: there's barely any protocol overhead between you calling a send function and the bytes going over the air.
- **Addressing is by MAC address, not IP.** You tell ESP-NOW about a "peer" using its Wi-Fi MAC address (every chip has a unique one). You can also use the special broadcast address `FF:FF:FF:FF:FF:FF`, which every nearby board (that is also running ESP-NOW) will receive — effectively a group chat with no server.
- **Messages are small and unordered.** One `esp_now_send()` carries at most **250 bytes** (`ESP_NOW_MAX_DATA_LEN`). There is no connection, no retries, and no ordering — if you need those, you build them yourself. For a short line of text between boards, 250 bytes is plenty.
- **It can be encrypted, but broadcast can't be.** One-to-one peers can use encryption; traffic sent to the broadcast address is always in the clear, since there's no single peer to share a key with.
- **Why would you use it?** Anywhere you want several nearby devices to react to each other instantly without any infrastructure: remote controls, synchronized lights, swarms of sensors, or — as in this assignment — a room full of workshop boards exchanging lines you type.

In this assignment we'll create a chatroom. You will send messages to other devices via the serial monitor - or control other people's LED!

> **Start a fresh project this time.** ESP-NOW doesn't need the Wi-Fi provisioning or OTA code from Assignments 3–4:

```bash
idf.py create-project esp_now_chat
cd esp_now_chat
idf.py set-target esp32c5
idf.py add-dependency "espressif/led_strip^3.0.3"
```

Copy [`chatroom.c`](../../../examples/C5/assignment-05-esp-now/main/chatroom.c) and [`chatroom.h`](../../../examples/C5/assignment-05-esp-now/main/chatroom.h) into your `main/` folder. 

Add `chatroom.c` and the LED component:

```cmake
idf_component_register(SRCS "esp_now_chat.c" "chatroom.c"
                       INCLUDE_DIRS "."
                       REQUIRES led_strip)
```

The new main file (``esp_now_chat.c``) starts as:

```c
#include <stdio.h>

void app_main(void)
{

}
```

### 1. Writing the Code

As before, let's build `main/esp_now_chat.c` up instead of pasting it all at once. The chat room calls three functions that you provide here: broadcast, send to one MAC, and remember a MAC as a peer. It never calls `esp_now_*` itself.

#### 1.1 Skeleton

```c
#include <stdio.h>

// (A) Headers and the broadcast address
// (B) Receive callback: hand the packet to the chat room
// (C) espnow_broadcast, espnow_send, espnow_add_peer

void app_main(void)
{
    // (D) Initialize NVS
    // (E) Turn the Wi-Fi radio on (station mode, no router)
    // (F) Initialize ESP-NOW, register the receive callback, add the broadcast peer
    // (G) Start the chat room
}
```

#### 1.2 Part (A): headers and the broadcast address

```c
#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "chatroom.h"

/* Exactly 8 ASCII characters, unique in the room. */
#define THIS_NODE_NAME "node0001" // TODO change to your liking - but keep 8 chars!
_Static_assert(sizeof(THIS_NODE_NAME) == CHATROOM_NAME_LEN + 1,
               "THIS_NODE_NAME must be exactly 8 characters");

static const char *TAG = "espnow";

static const uint8_t s_broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
```
Broadcast MAC address is `FF:FF:FF:FF:FF:FF` in ESP-NOW protocol. Every other ESP-NOW device listens to messages sent for this address.

#### 1.3 Part (B): the receive callback

ESP-NOW calls this from the Wi-Fi task whenever a packet arrives. The payload is a raw buffer with a length, not a C string. We don't interpret it here — `chatroom_on_recv` does that, and it also reads `src_addr`, the MAC of whoever sent the packet.

```c
static void on_data_recv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len)
{
    if (recv_info == NULL || recv_info->src_addr == NULL) {
        return;
    }
    chatroom_on_recv(recv_info->src_addr, data, data_len);
}
```

#### 1.4 Part (C): send, and add a peer

`esp_now_send()` hands `len` bytes to the radio and returns. It does not wait for anyone to receive them — there is no connection to wait on. The same call is a broadcast or a direct message depending only on which MAC you pass.

A direct send works only for a MAC that was registered with `esp_now_add_peer()`. The chat room does that the first time it hears a new board. `channel = 0` means "whatever channel the radio is already on". Broadcast peers cannot set `encrypt`.

```c
esp_err_t espnow_broadcast(const uint8_t *data, size_t len)
{
    return espnow_send(s_broadcast_mac, data, len);
}

esp_err_t espnow_send(const uint8_t mac[6], const uint8_t *data, size_t len)
{
    return esp_now_send(mac, data, len);
}

esp_err_t espnow_add_peer(const uint8_t mac[6])
{
    if (esp_now_is_peer_exist(mac)) {
        return ESP_OK;
    }

    esp_now_peer_info_t peer = { 0 };
    memcpy(peer.peer_addr, mac, 6);
    peer.channel = 0;   /* current channel */
    peer.encrypt = false;
    return esp_now_add_peer(&peer);
}
```

#### 1.5 Parts (D)–(G): `app_main`

ESP-NOW needs the Wi-Fi peripheral on, in station mode, even though we never join a router. `esp_wifi_start()` turns the radio on. NVS comes first, same as in the Wi-Fi assignments, because the Wi-Fi driver stores calibration data there.

The broadcast peer has to be added before the first send, or `esp_now_send()` to `FF:FF:FF:FF:FF:FF` fails with "peer not found". `chatroom_start()` is the keyboard and the code that prints received lines; everything it transmits goes back through part (C).

```c
void app_main(void)
{
    // (D) Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // (E) Turn the Wi-Fi radio on (station mode, no router)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // (F) Initialize ESP-NOW, register the receive callback, add the broadcast peer
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_recv));

    esp_now_peer_info_t broadcast = { 0 };
    memcpy(broadcast.peer_addr, s_broadcast_mac, 6);
    broadcast.channel = 0;
    broadcast.encrypt = false; /* broadcast traffic cannot be encrypted */
    ESP_ERROR_CHECK(esp_now_add_peer(&broadcast));

    // (G) Start the chat room
    chatroom_start(THIS_NODE_NAME);
    ESP_LOGI(TAG, "ESP-NOW is up. Type in this monitor.");
}
```

#### 1.6 Complete code

[`examples/C5/assignment-05-esp-now`](../../../examples/C5/assignment-05-esp-now/) has this program in `main/main.c`, plus the `chatroom.c` / `chatroom.h` pair. The example names that file `main.c`; `create-project` named yours `esp_now_chat.c`.

### 2. Build, Flash, and Monitor

```bash
idf.py build flash monitor
```

`idf.py monitor` is a serial terminal in both directions. Lines the board prints show up here, and keys you press are sent to the board (except `Ctrl-]`, which quits, and `Ctrl-T`, which opens the monitor menu). The board echoes what you type. A message from someone else can print in the middle of that line; the board still receives only your keystrokes, and Enter sends the line you typed.

Flash a second board with a different name, for example `node0001` on yours and `node0002` on theirs.

- Type `hello` and press Enter. Their monitor prints `User node0001: hello`.
- Type `red` (or `green`, `blue`, `white`, `on`, `off`). Their LED changes, and the monitor prints `User node0001 set your LED to 20,0,0` (the numbers, not the word). Each level is 0–255. Those words are the `COMMAND_RED` (and similar) lines at the top of `chatroom.c`; a new word is a new row in that table.
- After their board has sent anything, type `node0002:hello` or `node0002:blue` to reach only that board. Until then the monitor says there is no such contact. ESP-NOW keeps 20 peers, and one slot is the broadcast address, so the room remembers 19 names.

A line that is not ASCII is refused. A packet longer than 250 bytes is cut to fit before it is sent. A board does not receive its own broadcast.

## Next step

Let's look at one final feature of the ESP32-C5: The Low-Power core!

[Assignment 6: LP-core](../assignment-6/)
