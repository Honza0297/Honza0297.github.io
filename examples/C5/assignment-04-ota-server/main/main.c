#include <stdlib.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_partition.h"
#include "esp_image_format.h"

#define WIFI_SSID     "ESP32-WS"
#define WIFI_PASSWORD "ESP32-WS"

static const char *TAG = "ota_server";

static const esp_partition_t *s_fw;
static size_t s_fw_len;

static esp_err_t firmware_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Sending firmware (%u bytes)", (unsigned)s_fw_len);

    uint8_t *buf = malloc(4096);
    if (buf == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "out of memory");
        return ESP_ERR_NO_MEM;
    }

    size_t offset = 0;
    esp_err_t err = ESP_OK;
    while (offset < s_fw_len) {
        size_t n = s_fw_len - offset;
        if (n > 4096) {
            n = 4096;
        }
        err = esp_partition_read(s_fw, offset, buf, n);
        if (err != ESP_OK) {
            break;
        }
        err = httpd_resp_send_chunk(req, (const char *)buf, n);
        if (err != ESP_OK) {
            break;
        }
        offset += n;
    }
    free(buf);

    if (err == ESP_OK) {
        err = httpd_resp_send_chunk(req, NULL, 0);
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send firmware at offset %u", (unsigned)offset);
    } else {
        ESP_LOGI(TAG, "Firmware sent");
    }
    return err;
}

// The download URL is fixed at 192.168.0.100. DHCP on this router hands that
// address to whoever asks first, so claim it as soon as the link is up.
static void set_static_ip(esp_netif_t *netif)
{
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop DHCP client");
        return;
    }

    esp_netif_ip_info_t ip = { 0 };
    ip.ip.addr = ESP_IP4TOADDR(192, 168, 0, 100);
    ip.netmask.addr = ESP_IP4TOADDR(255, 255, 255, 0);
    ip.gw.addr = ESP_IP4TOADDR(192, 168, 0, 1);
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set static IP");
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        set_static_ip(arg);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Disconnected from \"%s\", retrying", WIFI_SSID);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Connected. Firmware URL: http://" IPSTR "/firmware.bin",
                 IP2STR(&event->ip_info.ip));
    }
}

static void start_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192;
    config.send_wait_timeout = 60;
    config.recv_wait_timeout = 60;

    httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(httpd_start(&server, &config));

    httpd_uri_t fw = {
        .uri = "/firmware.bin",
        .method = HTTP_GET,
        .handler = firmware_get_handler,
    };
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &fw));
}

// Read the length of the app image stored in the fw partition. The partition
// is larger than the file; trailing 0xFF must not be sent to the OTA client.
static void prepare_image(void)
{
    s_fw = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "fw");
    if (s_fw == NULL) {
        ESP_LOGE(TAG, "No 'fw' partition. Flash this project with idf.py flash.");
        abort();
    }

    esp_partition_pos_t pos = {
        .offset = s_fw->address,
        .size = s_fw->size,
    };
    esp_image_metadata_t meta = { 0 };
    ESP_ERROR_CHECK(esp_image_get_metadata(&pos, &meta));
    s_fw_len = meta.image_len;
    ESP_LOGI(TAG, "Stored firmware image is %u bytes", (unsigned)s_fw_len);
}

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    prepare_image();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, sta_netif));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, // "ESP32-WS" is 8 characters, the WPA2 minimum
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    // Modem sleep misses beacons on this AP and the server drops offline.
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    start_server();
}
