#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>  // Or ST7789
#include <Adafruit_BMP280.h>
#include <SPI.h>

// TFT Pins
#define TFT_CS   5
#define TFT_DC   17  // A0 pin on TFT
#define TFT_RST  16
#define TFT_BL 14

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// BMP280
Adafruit_BMP280 bmp;

// Button
#define BUTTON_PIN 4

bool displayOn = false;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // TFT setup
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
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
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (!displayOn) {
      displayOn = true;  // Track display state
    }
 
    float temp = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0;
    digitalWrite(TFT_BL, HIGH);
    Serial.println("Button Pressed");

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

    delay(2000);  // Wait before next reading
  } else {
    if (displayOn) {
      displayOn = false;
      Serial.println("Button Released - Turning off screen");
      tft.fillScreen(ST77XX_BLACK);  // Clear screen (simulate off)
      digitalWrite(TFT_BL, LOW);
      delay(10);
    }
    delay(200);
  }
}
