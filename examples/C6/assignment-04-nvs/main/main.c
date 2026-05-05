#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    nvs_handle_t handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &handle));
    ESP_ERROR_CHECK(nvs_set_i32(handle, "boot_count", 1));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
    printf("Workshop example: assignment 4 — NVS write/read skeleton.\n");
}
