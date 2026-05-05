/*
 * Assignment 5 — Wi-Fi provisioning
 *
 * Students create this project directly from the espressif/network_provisioning
 * component example:
 *
 *   idf.py create-project-from-example \
 *       "espressif/network_provisioning^1.2.0:wifi_prov"
 *
 * No source changes are required — just set the target, erase flash, build,
 * and flash.  This file is a compile-check stub that verifies the component
 * headers resolve for esp32c5.
 */

#include <stdio.h>
#include "network_provisioning/manager.h"
#include "network_provisioning/scheme_ble.h"

void app_main(void)
{
    network_prov_mgr_config_t config = {
        .scheme = network_prov_scheme_ble,
        .scheme_event_handler = NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BT,
    };
    ESP_ERROR_CHECK(network_prov_mgr_init(config));
    network_prov_mgr_deinit();
    printf("Workshop example: assignment 5 — network_provisioning headers resolve for this target.\n");
}
