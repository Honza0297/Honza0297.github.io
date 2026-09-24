---
title: "ESP-IDF and ESP32-C5 Workshop - Assignment 4: OTA Updates"
date: 2024-06-30T00:00:00+01:00
layout: default
---

## Assignment 4: Over-The-Air (OTA) Updates

---

> **Note:** This is an optional chapter. If time is running short, you may skip directly to [Assignment 5: ESP-NOW Chat Room](../assignment-5/).

In a real-world deployment, you won't have physical access to a USB port to flash new firmware. Over-The-Air (OTA) updates allow you to download and install new firmware over a network connection like Wi-Fi.

### 1. Partition Tables

To support OTA, the flash memory of the ESP32-C5 must be divided into specific regions called partitions. A typical OTA partition table includes:
- **nvs**: Non-Volatile Storage for Wi-Fi credentials and calibration data.
- **otadata**: Stores information about which app partition is currently active.
- **ota_0**: The factory or primary app partition.
- **ota_1**: The secondary app partition where the new firmware will be downloaded.

When an OTA update occurs, the device downloads the new firmware into the inactive partition (e.g., `ota_1`). If the download is successful, it updates the `otadata` partition to point to `ota_1` and reboots.

We define this layout in a file named `partitions.csv` in our project root.

### 2. Configuration via menuconfig

To tell ESP-IDF to use our custom partition table, run:

```bash
idf.py menuconfig
```

Navigate to:
- **Partition Table**
  - **Partition Table**: Select `Custom partition table CSV`
  - **Custom partition CSV file**: Type `partitions.csv`
  - Leave other settings as default.

Press `Q` to quit and `Y` to save.

### 3. Writing the Code

The `esp_https_ota` component provides a simple API for performing secure updates over HTTPS.

In your `main.c`, you can define a task that attempts to update the firmware:

```c
#include "esp_https_ota.h"

// ... Wi-Fi connection code from previous assignment ...

void ota_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Starting OTA update task");

    esp_http_client_config_t config = {
        .url = "https://example.com/firmware.bin",
        // In production, always provide a certificate!
        // .cert_pem = (char *)server_cert_pem_start,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    ESP_LOGI(TAG, "Attempting to download update from %s", config.url);
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA Succeed, Rebooting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "Firmware upgrade failed");
    }
    vTaskDelete(NULL);
}
```

By changing the URL to a valid firmware binary hosted on an HTTPS server, the ESP32-C5 will download the file, verify it, flash it to the alternate partition, and restart!

### 4. Rollback (Bonus)

ESP-IDF also supports App Rollback. If a newly flashed firmware panics or fails to connect to the server within a certain time frame, the bootloader will mark it as invalid and boot back into the previous, working firmware partition.

## Next step

Let's dive into an entirely different wireless technology: ESP-NOW!

[Assignment 5: ESP-NOW Chat Room](../assignment-5/)
