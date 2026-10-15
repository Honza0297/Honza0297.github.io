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
/* Wrong length fails the build, not at runtime. */
#define THIS_NODE_NAME "node0001"
_Static_assert(sizeof(THIS_NODE_NAME) == CHATROOM_NAME_LEN + 1,
               "THIS_NODE_NAME must be exactly 8 characters");

static const char *TAG = "espnow";

static const uint8_t s_broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static void on_data_recv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len)
{
    if (recv_info == NULL || recv_info->src_addr == NULL) {
        return;
    }
    /* Runs in the Wi-Fi task: hand over fast and return. */
    chatroom_on_recv(recv_info->src_addr, data, data_len);
}

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
    /* Adding twice fails; skip known boards. */
    if (esp_now_is_peer_exist(mac)) {
        return ESP_OK;
    }

    esp_now_peer_info_t peer = { 0 };
    memcpy(peer.peer_addr, mac, 6);
    peer.channel = 0;   /* current channel */
    peer.encrypt = false; /* no encryption in this workshop */
    return esp_now_add_peer(&peer);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    /* Flash was erased or the layout changed; start NVS fresh. */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* ESP-NOW uses the Wi-Fi radio, but it does not connect to a router. */
    ESP_ERROR_CHECK(esp_netif_init()); /* TCP/IP stack */
    ESP_ERROR_CHECK(esp_event_loop_create_default()); /* the loop Wi-Fi posts events to */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); /* station mode, but we never join a router */
    ESP_ERROR_CHECK(esp_wifi_start()); /* radio on; no connection yet */

    ESP_ERROR_CHECK(esp_now_init());
    /* Every packet lands in on_data_recv. */
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_recv));

    esp_now_peer_info_t broadcast = { 0 };
    memcpy(broadcast.peer_addr, s_broadcast_mac, 6);
    broadcast.channel = 0;
    broadcast.encrypt = false; /* broadcast traffic cannot be encrypted */
    ESP_ERROR_CHECK(esp_now_add_peer(&broadcast));

    chatroom_start(THIS_NODE_NAME);
    ESP_LOGI(TAG, "ESP-NOW is up. Type in this monitor.");
}
