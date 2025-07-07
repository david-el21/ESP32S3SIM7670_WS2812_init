idf_component_register(SRCS "main.c" "ws2812_control.c"
                    PRIV_REQUIRES esp_wifi nvs_flash led_strip
                    INCLUDE_DIRS ".")