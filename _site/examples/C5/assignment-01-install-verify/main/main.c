#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    printf("Workshop example: assignment 1 — ESP-IDF environment looks OK.\n");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
