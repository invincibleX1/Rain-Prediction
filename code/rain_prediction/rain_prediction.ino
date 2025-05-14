#include <Adafruit_BMP280.h>
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


// --- Pressure History Settings ---
#define MAX_READINGS 60

// --- Components ---
Adafruit_BMP280 bmp;
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

  // Init BMP280
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not found!");
    delay(2000);
    return;
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
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

    esp_sleep_enable_timer_wakeup(10 * 60 * 1000000ULL);                    // 10 minutes
    esp_deep_sleep_start();
  }

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



