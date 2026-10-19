#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "qrcode.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"

static const char *TAG = "provisioning";

// Proof-of-possession: the app gets it from the QR code, so the user never types it
#define PROV_POP "abcd1234"

// True while the provisioning manager runs. It drives esp_wifi_connect() itself, so we must stay out of its way.
static volatile bool s_prov_active = false;

// Reconnect back-off: wait 1 s, 2 s, 4 s ... up to 30 s between attempts, so an unreachable router is not hammered.
#define RECONNECT_MAX_DELAY_MS 30000
static int s_retry_count = 0;
static esp_timer_handle_t s_reconnect_timer;

static void reconnect_cb(void *arg)
{
    esp_wifi_connect();
}

// Print the QR code the ESP BLE Prov app scans (device name, PoP and transport).
// How the QR code is rendered is not important for this workshop.
static void print_qr(const char *service_name)
{
    char payload[150];
    snprintf(payload, sizeof(payload),
             "{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"ble\"}",
             service_name, PROV_POP);
    ESP_LOGI(TAG, "Scan this QR code with the ESP BLE Prov app:");
    esp_qrcode_config_t qr_cfg = ESP_QRCODE_CONFIG_DEFAULT();
    esp_qrcode_generate(&qr_cfg, payload); /* draws the QR code into the monitor output */
}

// Event handler for Wi-Fi events
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV: {
                /* event_data carries the SSID and password the phone just sent. */
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                /* DEMO ONLY: never log passwords in production. It is printed here so you can see what the phone sent. */
                ESP_LOGI(TAG, "Received Wi-Fi credentials\n\tSSID     : %s\n\tPassword : %s (do not log passwords in plaintext ;))",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);
                break;
            }
            case WIFI_PROV_CRED_FAIL:
                /* Wrong password or AP not found. Forget the bad credentials, otherwise the board
                   would reboot into "already provisioned" and never offer provisioning again. */
                ESP_LOGE(TAG, "Provisioning failed. Check the password and try again.");
                wifi_prov_mgr_reset_sm_state_on_failure();
                break;
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG, "Provisioning successful");
                break;
            case WIFI_PROV_END:
                ESP_LOGI(TAG, "Provisioning completed. Deinitializing manager.");
                s_prov_active = false;
                wifi_prov_mgr_deinit(); /* manager did its job; free it */
                break;
            default:
                break;
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        /* Radio is on; start connecting (during provisioning the manager does it). */
        if (!s_prov_active) {
            esp_wifi_connect();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        /* event_data carries the address DHCP gave us. */
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_count = 0; /* connected; the next outage starts with a short delay again */
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (!s_prov_active) {
            /* dropped; try again, waiting a bit longer after every failure */
            uint32_t delay_ms = 1000U << (s_retry_count < 5 ? s_retry_count : 5);
            if (delay_ms > RECONNECT_MAX_DELAY_MS) {
                delay_ms = RECONNECT_MAX_DELAY_MS;
            }
            s_retry_count++;
            ESP_LOGI(TAG, "Disconnected. Connecting to the AP again in %lu ms...", (unsigned long)delay_ms);
            esp_timer_start_once(s_reconnect_timer, (uint64_t)delay_ms * 1000);
        }
    }
}

void app_main(void)
{
    // 1. Initialize NVS (needed for Wi-Fi and Provisioning to save data)
    esp_err_t ret = nvs_flash_init();
    /* Flash was erased or the layout changed; start NVS fresh. */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize networking
    ESP_ERROR_CHECK(esp_netif_init()); /* TCP/IP stack */
    ESP_ERROR_CHECK(esp_event_loop_create_default()); /* the loop Wi-Fi posts events to */
    esp_netif_create_default_wifi_sta(); /* default station interface, no IP yet */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 3. Register event handlers
    const esp_timer_create_args_t reconnect_args = { .callback = &reconnect_cb, .name = "wifi_reconnect" };
    ESP_ERROR_CHECK(esp_timer_create(&reconnect_args, &s_reconnect_timer)); /* used by the reconnect back-off */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL)); /* only the got-address event */

    // 4. Start Wi-Fi Provisioning Manager
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble, /* talk to the phone over BLE */
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM /* frees BLE memory when done */
    };
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    bool provisioned = false;
    /* Checks NVS: did we already get credentials before? */
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    if (!provisioned) {
        ESP_LOGI(TAG, "Starting provisioning");
        // Unique name per board (last 3 bytes of the MAC), so many boards can be provisioned in one room
        uint8_t mac[6];
        ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));
        char service_name[16];
        snprintf(service_name, sizeof(service_name), "PROV_%02X%02X%02X", mac[3], mac[4], mac[5]);

        s_prov_active = true;
        // Security 1: encrypted session, authenticated by the proof-of-possession from the QR code
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(WIFI_PROV_SECURITY_1, PROV_POP, service_name, NULL));
        print_qr(service_name);
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi");
        wifi_prov_mgr_deinit(); /* never started; just drop the manager */
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }
}
