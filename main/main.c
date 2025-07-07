#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ws2812_control.h"

#define WS2812_GPIO 38  // Schimbă cu pinul folosit

void app_main(void) {
    ws2812_init(WS2812_GPIO);

    while (1) {
        ws2812_set_color(255, 0, 0);  // 🟢 Verde
        vTaskDelay(pdMS_TO_TICKS(1000));

        ws2812_set_color(0, 255, 0);  // 🔴 Roșu
        vTaskDelay(pdMS_TO_TICKS(1000));

        ws2812_set_color(0, 0, 255);  // 🔵 Albastru
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}