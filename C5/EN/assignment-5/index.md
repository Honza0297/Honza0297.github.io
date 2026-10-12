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
- **Messages are small and unordered.** A single ESP-NOW packet caps out around 250 bytes, and there's no built-in concept of a connection, retries, or ordering — if you need those, you build them yourself on top. For a "chat room" of short commands between boards, this is more than enough.
- **It can be encrypted, but broadcast can't be.** One-to-one peers can use encryption; traffic sent to the broadcast address is always in the clear, since there's no single peer to share a key with.
- **Why would you use it?** Anywhere you want several nearby devices to react to each other instantly without any infrastructure: remote controls, synchronized lights or decorations, swarms of sensors reporting to each other, or — as in this assignment — a room full of workshop boards reacting to each other's button presses.

In this assignment, we will turn our workshop room into a giant ESP-NOW Chat Room! Pressing the button on your board broadcasts a command, and every other board that receives it changes its LED accordingly.

> **Start a fresh project this time.** ESP-NOW doesn't need the Wi-Fi provisioning or OTA/partition-table code from Assignments 3-4, so create a new project (`idf.py create-project`, `idf.py set-target esp32c5`) rather than continuing the old one. You will still need the `led_strip` dependency (`idf.py add-dependency "espressif/led_strip^3.0.3"`) — but **keep your button and LED wired up exactly as they were in Assignment 4**, we're reusing that hardware as-is.

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

### 2. Adding the Broadcast Peer

To broadcast to everyone in the room, we add a peer with the broadcast MAC address `FF:FF:FF:FF:FF:FF`.

```c
esp_now_peer_info_t peer_info = {};
memset(&peer_info, 0, sizeof(peer_info));
uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
memcpy(peer_info.peer_addr, broadcast_mac, 6);
peer_info.channel = 0;      // Use current channel
peer_info.encrypt = false;  // No encryption for broadcast

ESP_ERROR_CHECK(esp_now_add_peer(&peer_info));
```

### 3. Receiving Commands

We register a callback function that executes whenever an ESP-NOW packet is received. This function will parse the incoming string and change our LED color — and, since it keeps whatever color it's set to until the next command, your board becomes a little display of "the last command anyone sent".

```c
static void on_data_recv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len)
{
    // data might not be null terminated, so let's copy it safely
    char msg[32] = {0};
    int len = data_len < 31 ? data_len : 31;
    memcpy(msg, data, len);

    ESP_LOGI(TAG, "Received: %s", msg);

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

// In app_main:
ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_recv));
```

`led_set_color` is a tiny helper (shown in full below) that just calls `led_strip_set_pixel` + `led_strip_refresh` — the same pair of calls you've been using since Assignment 2.

### 4. Sending Commands with the Button

Instead of broadcasting on a timer, let's reuse the button from Assignment 4: each press sends the next command from a small list, round-robin style. We also flash the LED white for a moment to confirm the send actually went out (or red, if it failed):

```c
static const char *commands[] = {"COLOR RED", "COLOR GREEN", "LED ON", "LED OFF"};
#define NUM_COMMANDS (sizeof(commands) / sizeof(commands[0]))

static void send_next_command(void)
{
    static int next = 0;
    const char *msg = commands[next];
    next = (next + 1) % NUM_COMMANDS;

    uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_err_t result = esp_now_send(broadcast_mac, (const uint8_t *)msg, strlen(msg));
    if (result == ESP_OK) {
        led_flash(20, 20, 20, 150); // white flash: sent
    } else {
        led_flash(20, 0, 0, 300); // red flash: send failed
    }
}
```

`send_next_command` is called from the same kind of button-polling task you wrote in Assignment 4 (`button_task`, debounced the same way) — we're not repeating it here since it's identical, just notifying a different function.

### 5. Build, Flash, and Monitor

Make sure you have the `led_strip` component added:
```bash
idf.py add-dependency "espressif/led_strip^3.0.3"
```

Then build and flash:
```bash
idf.py build flash monitor
```

Your LED should sit at a dim blue ("listening") when idle. Work with the person next to you: press your button to broadcast a color, and watch their board latch onto it — then let them do the same to you!

### Complete code

Everything above, wired together with the LED helpers and the button task from Assignment 4, lives at [`examples/C5/assignment-05-esp-now`](../../../examples/C5/assignment-05-esp-now/) if you'd like the full file to copy or compare against.

## Next step

Let's look at one final feature of the ESP32-C5: The Low-Power core!

[Assignment 6: LP-core](../assignment-6/)
