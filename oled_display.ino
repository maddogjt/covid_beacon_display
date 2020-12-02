#include "oled_display.h"

#if defined(ENABLE_OLED)
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_I2C_SDA 5
#define OLED_I2C_SCL 4

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
// GPIO5=SDA; GPIO4=SCL

// TwoWire OAdafruit_SSD1306ledI2C;
Adafruit_SSD1306 gDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET);

void oled_setup(){
    Wire1.begin(OLED_I2C_SDA, OLED_I2C_SCL);

    if (!gDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("failed to begin oled");
    }

    gDisplay.clearDisplay();

    gDisplay.setTextSize(2);
    gDisplay.setTextColor(WHITE);
    gDisplay.setCursor(0, 10);
    gDisplay.printf("C19\nBeacon\nDetector!\n");
    gDisplay.display();
}

void oled_loop()
{

}

#endif // defined(ENABLE_OLED)