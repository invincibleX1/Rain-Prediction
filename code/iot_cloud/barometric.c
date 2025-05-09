#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include "thingProperties.h"

#define BUTTON_PIN  2  // Change this to your actual button pin
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_BMP280 bmp;
bool buttonState = false;

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // Initialize OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed");
        while (1);
    }
    display.clearDisplay();
    
    // Initialize BMP280 sensor
    if (!bmp.begin(0x76)) {
        Serial.println("BMP280 sensor not found");
        while (1);
    }
    
    // Connect to WiFi & Arduino IoT Cloud
    initProperties();
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);
}

void loop() {
    ArduinoCloud.update();
    buttonState = digitalRead(BUTTON_PIN) == LOW;
    
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F; // hPa
    
    if (buttonState) {
        // Display data on OLED when button is pressed
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 10);
        display.print("Temp: ");
        display.print(temperature);
        display.println(" C");
        display.print("Pressure: ");
        display.print(pressure);
        display.println(" hPa");
        display.display();
    } else {
        // Send data to Arduino IoT Cloud when idle
        tempCloud = temperature;
        pressureCloud = pressure;
    }
    delay(500);
}
