#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Wire.h>
#include <esp_sleep.h>
#include <DHT11.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Your Wi-Fi credentials
const char* ssid = "COSMOTE-989810";
const char* password = "69786762248281985263";

// Google Apps Script URL (replace with your generated URL)
const char* serverName = "https://script.google.com/macros/s/AKfycbybCCg4eVdEeiegs4Sg1la2WAdmTb50N8G0uoCnZTd-uJ8NXg2N6w_aiB9-uGrwCIqX/exec";
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

RTC_DATA_ATTR float lastPressure = 0.0;
RTC_DATA_ATTR float lastTemperature = 0.0;
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
      WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Read current pressure and temperature
    float pressure = bmp.readPressure() / 100.0;  // hPa
    float temperatureB = bmp.readTemperature();    // °C
    float temperatureD = dht11.readTemperature();  // °C
    float humidity = dht11.readHumidity();
    float temperature = 0.3*temperatureB + 0.7*temperatureD;
    Serial.print("MEASURED\n");

    // Store last values in the RTC
    lastPressure = pressure;
    lastTemperature = temperature;
    lastTempBMP = temperatureB;
    lastTempDHT = temperatureD;
    lastHumidity = humidity;

    // Print all stored values in pressureHistory[]
    Serial.print("Data: [");
    Serial.print(lastPressure);
    Serial.print(" hPa, ");
    Serial.print(lastTemperature);
    Serial.print(" C, ");
    Serial.print(lastHumidity);
    Serial.println(" %]");


    sendDataToGoogleSheets(lastPressure,lastTemperature, lastHumidity);
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

    tft.print("Pressure: ");
    tft.print(lastPressure, 1);
    tft.println(" hPa");

    tft.print("Temp: ");
    tft.print(lastTemperature, 1);
    tft.println(" C");

    tft.print("Humidity: ");
    tft.print(lastHumidity, 1);
    tft.println(" %");

    tft.print("TempDHT: ");
    tft.print(lastTempDHT, 1);
    tft.println(" C");

    tft.print("TempBMP: ");
    tft.print(lastTempBMP, 1);
    tft.println(" C");



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

//Send data to Google Sheets
void sendDataToGoogleSheets(float pressure, float temp, float hum) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);  // Specify your Google Apps Script URL
    http.addHeader("Content-Type", "application/json");

    // Create JSON payload with pressure, temperature, and humidity
    String payload = "{";
    payload += "\"pressure\": " + String(pressure) + ", ";
    payload += "\"temperature\": " + String(temp) + ", ";
    payload += "\"humidity\": " + String(hum);
    payload += "}";

    // Debug output
    Serial.print("Payload: ");
    Serial.println(payload);

    // Send the POST request
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.print("Data sent successfully! Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending data. HTTP response code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
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
