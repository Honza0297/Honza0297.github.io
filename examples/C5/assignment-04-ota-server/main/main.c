#include <stdlib.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_partition.h"
#include "esp_image_format.h"

#define AP_SSID     "ESP32"
#define AP_PASSWORD "ESP32"

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
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = sizeof(AP_SSID) - 1,
            .password = AP_PASSWORD,
            .channel = 1,
            .authmode = WIFI_AUTH_WEP, // "ESP32" is 5 characters; WPA2 needs 8
            .max_connection = 10,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    start_server();
    ESP_LOGI(TAG, "AP \"%s\" is up. Firmware URL: http://192.168.4.1/firmware.bin", AP_SSID);
}
