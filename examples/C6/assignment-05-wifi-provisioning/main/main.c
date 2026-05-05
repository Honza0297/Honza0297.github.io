#include <stdio.h>
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"

void app_main(void)
{
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
    };
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
    wifi_prov_mgr_deinit();
    printf("Workshop example: assignment 5 — Wi-Fi provisioning init skeleton.\n");
}
