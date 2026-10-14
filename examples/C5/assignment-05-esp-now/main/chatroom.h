#pragma once

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

/* Exactly 8 ASCII characters, so a name is a fixed-width field. */
#define CHATROOM_NAME_LEN 8

/* Start the keyboard task and the task that applies received packets.
 * Call this after ESP-NOW itself is initialized.
 * `node_name` is exactly CHATROOM_NAME_LEN ASCII characters. */
void chatroom_start(const char *node_name);

/* Hand one received ESP-NOW payload to the chat room.
 * `data` is not a C string; `len` is the number of bytes. */
void chatroom_on_recv(const uint8_t src_mac[6], const uint8_t *data, int len);

/* Implemented in main.c. These are the only ESP-NOW operations the chat room uses. */
esp_err_t espnow_broadcast(const uint8_t *data, size_t len);
esp_err_t espnow_send(const uint8_t mac[6], const uint8_t *data, size_t len);
esp_err_t espnow_add_peer(const uint8_t mac[6]);
