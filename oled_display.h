#pragma once

#define ENABLE_OLED

#if defined(ENABLE_OLED)

#include <Adafruit_SSD1306.h>

void oled_setup();
void oled_loop();

extern Adafruit_SSD1306 gDisplay;

#define IF_OLED(...) __VA_ARGS__

#else

#define IF_OLED(...) 

#endif