#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Wire.h>
#include <esp_sleep.h>
#include <DHT11.h>
// --- TFT & Sensor Pins ---
#define TFT_CS     5
#define TFT_DC     17
#define TFT_RST    16

// --- Wake Button Pin (must be RTC-capable) ---
#define WAKE_BUTTON_PIN 14
#define SCREEN_POWER_PIN 4

// --- Pressure History Settings ---
#define MAX_READINGS 60
#define DROP_THRESHOLD 2.5  // hPa drop for "Rain is Coming"

// --- Components ---
Adafruit_BMP280 bmp;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
DHT11 dht11(15);
// --- RTC-Persistent Data ---
RTC_DATA_ATTR float pressureHistory[MAX_READINGS];
RTC_DATA_ATTR int pressureIndex = 0;
RTC_DATA_ATTR bool bufferFull = false;
RTC_DATA_ATTR float lastPressure = 0.0;
RTC_DATA_ATTR float lastTemperature = 0.0;
RTC_DATA_ATTR bool lastRainComing = false;
RTC_DATA_ATTR float lastTempBMP = 0.0;
RTC_DATA_ATTR float lastTempDHT = 0.0;
RTC_DATA_ATTR float lastHumidity = 0.0;

void setup() {
  Serial.begin(115200);
  delay(100);

  // Detect wake reason
  esp_sleep_wakeup_cause_t wakeReason = esp_sleep_get_wakeup_cause();
  bool screenRequested = (wakeReason == ESP_SLEEP_WAKEUP_EXT0);

  // Init BMP280
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not found!");
    delay(2000);
    return;
  }

  pinMode(SCREEN_POWER_PIN, OUTPUT);
  
  

  // If it's a timer wakeup, gather measurements
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
    // Read current pressure and temperature
    float pressure = bmp.readPressure() / 100.0;  // hPa
    float temperatureB = bmp.readTemperature();    // °C
    float temperatureD = dht11.readTemperature();  // °C
    float humidity = dht11.readHumidity();
    float temperature = 0.3*temperatureB + 0.7*temperatureD;
    Serial.print("MEASURED\n");
    // Store to circular buffer
    pressureHistory[pressureIndex] = pressure;
    pressureIndex = (pressureIndex + 1) % MAX_READINGS;
    if (pressureIndex == 0) bufferFull = true;

    // Detect pressure drop
    bool rainComing = false;
    if (bufferFull) {
      int pastIndex = (pressureIndex + 1) % MAX_READINGS;
      float pastPressure = pressureHistory[pastIndex];
      if ((pastPressure - pressure) >= DROP_THRESHOLD) {
        rainComing = true;
      }
    }

    // Store last values
    lastPressure = pressure;
    lastTemperature = temperature;
    lastRainComing = rainComing;
    lastTempBMP = temperatureB;
    lastTempDHT = temperatureD;
    lastHumidity = humidity;

    // Print all stored values in pressureHistory[]
    Serial.print("Pressure History: [");
    for (int i = 0; i < MAX_READINGS; i++) {
      Serial.print(pressureHistory[i], 1);
      if (i < MAX_READINGS - 1) {
        Serial.print(", ");
      }
    }
    Serial.println("]");

    // Prepare for next wake
    pinMode(WAKE_BUTTON_PIN, INPUT_PULLDOWN);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKE_BUTTON_PIN, 1);   // wake when HIGH
    esp_sleep_enable_timer_wakeup(60 * 1000000);                    // 60 seconds
    esp_deep_sleep_start();
  }

  // If button wakeup, show on screen
  if (screenRequested) {
    initScreen();
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);

    // Display the last measured values
    tft.print("Temp: ");
    tft.print(lastTemperature, 1);
    tft.println(" C");

    tft.print("Pressure: ");
    tft.print(lastPressure, 1);
    tft.println(" hPa");

    tft.print("TempDHT: ");
    tft.print(lastTempDHT, 1);
    tft.println(" °C");

    tft.print("TempBMP: ");
    tft.print(lastTempBMP, 1);
    tft.println(" C");

    tft.print("Humidity: ");
    tft.print(lastHumidity, 1);
    tft.println(" %");

    if (lastRainComing) {
      tft.setTextColor(ST77XX_RED);
      tft.println();
      tft.println("Rain is Coming!");
    }

    delay(5000);  // Display duration before sleeping
  }

  // Prepare for next wake
  pinMode(WAKE_BUTTON_PIN, INPUT_PULLDOWN);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKE_BUTTON_PIN, 1);   // wake when HIGH
  esp_sleep_enable_timer_wakeup(60 * 1000000);                    // 60 seconds
  esp_deep_sleep_start();
}

// Required by Arduino
void loop() {
  // Never called due to deep sleep
}

// TFT screen init
void initScreen() {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
  // Power on screen only if ESP woke from button
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
    digitalWrite(SCREEN_POWER_PIN, HIGH); // Power on TFT
  } else {
    digitalWrite(SCREEN_POWER_PIN, LOW);  // Optional: don't power screen on reset
  }
}
