idf_component_register(SRCS "esp_now_chat.c" "chatroom.c"
                       INCLUDE_DIRS "."
                       REQUIRES led_strip esp_wifi esp_event esp_netif nvs_flash)