#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>  // Or ST7789
#include <Adafruit_BMP280.h>
#include <SPI.h>

// TFT Pins
#define TFT_CS   5
#define TFT_DC   17  // A0 pin on TFT
#define TFT_RST  16


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// BMP280
Adafruit_BMP280 bmp;

void setup() {
  Serial.begin(115200);

  // TFT setup
  tft.initR(INITR_BLACKTAB);  // adjust as needed
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
  
  // Sensor setup
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not found!");
    while (1);
  }
  
}

void loop() {
  
    float temp = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0;
    

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 10);
    tft.println("Temperature");

    tft.setCursor(10, 30);
    tft.print(temp, 1);
    tft.println(" C");

    tft.setCursor(10, 60);
    tft.println("Pressure");

    tft.setCursor(10, 80);
    tft.print(pressure, 1);
    tft.println(" hPa");
  
    delay(20000);
  }

