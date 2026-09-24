---
title: "ESP-IDF and ESP32-C5 Workshop - Assignment 3: Wi-Fi & Provisioning"
date: 2024-06-30T00:00:00+01:00
layout: default
---

## Assignment 3: Wi-Fi & Provisioning

---

Wi-Fi is one of the most essential features for an IoT device. The **ESP32-C5** is special because it is Espressif's first SoC to support **dual-band Wi-Fi 6** (both 2.4 GHz and 5 GHz). The 5 GHz band is generally less crowded, leading to higher throughput and reduced interference.

In a real-world product, you cannot hardcode the Wi-Fi credentials (SSID and password) in your source code because every user has a different home network. Instead, you use **Wi-Fi Provisioning**. Provisioning allows a user to send their Wi-Fi credentials to the device using a mobile app over a secondary channel, typically Bluetooth Low Energy (BLE).

In this assignment, we will use the **ESP BLE Prov** mobile application to provision our board. The credentials will be saved in NVS (Non-Volatile Storage) so the board automatically reconnects after a reboot.

### 1. Enabling required components

We will need the `wifi_provisioning` component. Because it is part of ESP-IDF, we just need to ensure our code includes the right headers and initializes NVS properly.

### 2. Configuration via menuconfig

Some features must be configured before building. Run:

```bash
idf.py menuconfig
```

Navigate to:
- **Component config ---> Wi-Fi**
  - Ensure the software Wi-Fi configuration is enabled.
- **Component config ---> Wi-Fi Provisioning**
  - Notice the options available, but default settings work for us.
  
Press `Q` to quit and `Y` to save.

### 3. Writing the Code

Open your `main.c` and replace the contents with the following logic. This code initializes NVS, initializes the network interface, starts BLE provisioning, and then connects to Wi-Fi.

```c
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"

static const char *TAG = "provisioning";

// Event handler for Wi-Fi events
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV:
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(TAG, "Received Wi-Fi credentials\n\tSSID     : %s\n\tPassword : %s",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);
                break;
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG, "Provisioning successful");
                break;
            case WIFI_PROV_END:
                ESP_LOGI(TAG, "Provisioning completed. Deinitializing manager.");
                wifi_prov_mgr_deinit();
                break;
            default:
                break;
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
        esp_wifi_connect();
    }
}

void app_main(void)
{
    // 1. Initialize NVS (needed for Wi-Fi and Provisioning to save data)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize networking
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 3. Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // 4. Start Wi-Fi Provisioning Manager
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
    };
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    if (!provisioned) {
        ESP_LOGI(TAG, "Starting provisioning");
        // Name of the device you will see in the app
        char service_name[] = "PROV_C5_WORKSHOP";
        
        // Disable security for simplicity in workshop, but in real products always use WIFI_PROV_SECURITY_1 or 2
        wifi_prov_security_t security = WIFI_PROV_SECURITY_0;
        
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, NULL, service_name, NULL));
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi");
        wifi_prov_mgr_deinit();
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }
}
```

### 4. Build, Flash, and Monitor

```bash
idf.py build flash monitor
```

### 5. Using the ESP BLE Prov app

If this is your first time running the code on this board, it will start provisioning mode and advertise over BLE.

1. Open the **ESP BLE Prov** app on your phone.
2. Select **Provision Device**.
3. Choose **No Proof of Possession (Security 0)** (since we set `WIFI_PROV_SECURITY_0`).
4. Find `PROV_C5_WORKSHOP` in the list of devices and connect.
5. Select a Wi-Fi network from the list and enter the password. The ESP32-C5 supports both 2.4 GHz and 5 GHz networks!
6. Click Provision.

Look at your terminal! You should see the board receiving the credentials and connecting to your Wi-Fi network. If you restart the board (press the RESET button), it will automatically reconnect without needing the app again.

## Next step

Now that we have Wi-Fi working, we can explore remote updates!

[Assignment 4: OTA Updates](../assignment-4/)
