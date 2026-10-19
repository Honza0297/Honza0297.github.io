idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_event esp_netif nvs_flash esp_http_server bootloader_support esp_partition)

# firmware.bin is the student app, built with the log line set to "Running firmware v2!".
# idf.py flash writes it into the "fw" data partition; it is not linked into this app.
esptool_py_flash_to_partition(flash "fw" "${PROJECT_DIR}/firmware.bin")
