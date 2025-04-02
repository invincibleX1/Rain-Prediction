# Rain-Prediction
Rain real-time prediction with barometric sensor

Here is a short overview of the system.
The module will store 24h data locally, taking samples every 5-10 minutes (or less), and will enable wifi to send warning message when a significant change in barometric pressure is detected. The device will display the latest data on an oled display, while a button is pressed (for power saving reasons). The power will be a 1s rechargable Lithium battery or 3s alkaline (AA or AAA).

List of Components
*ESP32 C3 Super Mini
*GY-BMP280 sensor
*SH1106 1.3in oled display

*INR18650
*18650 battery holder
