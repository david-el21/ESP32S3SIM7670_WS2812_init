### 📁 ws2812_control/README.md

```markdown
# WS2812 RGB LED Control

Controlul unui LED RGB WS2812 folosind ESP32-S3 și biblioteca `led_strip.h` din ESP-IDF.

## Fișiere
- `ws2812.c` – inițializarea și controlul LED-ului
- `ws2812.h` – funcții publice

## Funcții disponibile
```c
void ws2812_init(gpio_num_t gpio);
void ws2812_set_color(uint8_t r, uint8_t g, uint8_t b);
## Exemplu
#include "ws2812.h"

void app_main() {
    ws2812_init(GPIO_NUM_18);
    ws2812_set_color(255, 0, 0); // roșu
}
Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.
