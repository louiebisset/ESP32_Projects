#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <U8g2lib.h>

#include "secrets.h"

// -------------------------
// Wi-Fi
// -------------------------

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WebServer server(80);

// -------------------------
// BME280
// -------------------------

Adafruit_BME280 bme;

int sensorType = 0; // 1 = BME280, 0 = None

// -------------------------
// OLED
// -------------------------

U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8g2(
  U8G2_R0,
  12,  // Clock
  11,  // Data
  10,  // CS
  13,  // DC
  14   // Reset
);

// -------------------------
// Webpage
// -------------------------
void handleRoot() {

  float t = bme.readTemperature();
  float h = bme.readHumidity();
  float p = bme.readPressure() / 100.0F;

  String webpage = R"rawliteral(
<!DOCTYPE html>
<html>

<head>

  <meta name="viewport" content="width=device-width, initial-scale=1">

  <title>ESP32 Sensor Dashboard</title>

  <style>

    * {
      box-sizing: border-box;
    }

    body {
      margin: 0;
      font-family: Arial, sans-serif;
      background: #111827;
      color: white;
    }

    .header {
      text-align: center;
      padding: 35px 20px 20px;
    }

    .header h1 {
      margin: 0;
      font-size: 32px;
    }

    .header p {
      color: #9ca3af;
      margin-top: 8px;
    }

    .container {
      max-width: 900px;
      margin: auto;
      padding: 20px;
    }

    .cards {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
      gap: 20px;
    }

    .card {
      background: #1f2937;
      border-radius: 16px;
      padding: 25px;
      text-align: center;
      box-shadow: 0 8px 20px rgba(0,0,0,0.25);
    }

    .card h2 {
      margin-top: 0;
      font-size: 18px;
      color: #9ca3af;
      font-weight: normal;
    }

    .value {
      font-size: 42px;
      font-weight: bold;
      margin: 15px 0;
    }

    .unit {
      font-size: 20px;
      color: #9ca3af;
    }

    .footer {
      text-align: center;
      color: #6b7280;
      margin-top: 30px;
      font-size: 14px;
    }

  </style>

</head>

<body>

  <div class="header">

    <h1>ESP32 Sensor Dashboard</h1>

    <p>BME280 Environmental Monitor</p>

  </div>

  <div class="container">

    <div class="cards">

      <div class="card">

        <h2>Temperature</h2>

        <div class="value">
          )rawliteral";

  webpage += String(t, 1);

  webpage += R"rawliteral(
          <span class="unit">&deg;C</span>
        </div>

      </div>


      <div class="card">

        <h2>Humidity</h2>

        <div class="value">
          )rawliteral";

  webpage += String(h, 1);

  webpage += R"rawliteral(
          <span class="unit">%</span>
        </div>

      </div>


      <div class="card">

        <h2>Pressure</h2>

        <div class="value">
          )rawliteral";

  webpage += String(p, 1);

  webpage += R"rawliteral(
          <span class="unit">hPa</span>
        </div>

      </div>

    </div>

    <div class="footer">

      ESP32-S3 &bull; BME280<br>
      Page refreshes every 5 seconds

    </div>

  </div>

</body>

</html>
)rawliteral";

  server.send(200, "text/html", webpage);
}


// -------------------------
// Setup
// -------------------------

void setup() {

  Serial.begin(115200);

  // -------------------------
  // Set up BME280 pins
  // -------------------------

  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);   // CS = HIGH -> I2C mode

  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);    // ADDR = LOW -> address 0x76

  delay(100);

  // -------------------------
  // Start I2C
  // -------------------------

  Wire.begin(8, 9);  // SDA = 8, SCL = 9

  // -------------------------
  // Find BME280
  // -------------------------

  if (bme.begin(0x76, &Wire) || bme.begin(0x77, &Wire)) {

    sensorType = 1;

    Serial.println("SUCCESS: Found BME280!");

  } else {

    Serial.println("ERROR: No sensor detected.");

  }

  // -------------------------
  // Start OLED
  // -------------------------

  u8g2.begin();

  // -------------------------
  // Connect to Wi-Fi
  // -------------------------

  Serial.println();
  Serial.print("Connecting to Wi-Fi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");

  }

  Serial.println();
  Serial.println("Wi-Fi connected!");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // -------------------------
  // Start web server
  // -------------------------

  server.on("/", handleRoot);

  server.begin();

  Serial.println("Web server started!");
}


// -------------------------
// Main loop
// -------------------------

void loop() {

  // Handle website requests
  server.handleClient();

  // -------------------------
  // Update OLED
  // -------------------------

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_tr);

  if (sensorType == 1) {

    float t = bme.readTemperature();
    float h = bme.readHumidity();
    float p = bme.readPressure() / 100.0F;

    u8g2.drawStr(0, 12, "BME280 MONITOR");

    u8g2.drawHLine(0, 16, 128);

    u8g2.setCursor(0, 32);
    u8g2.print("Temp:     ");
    u8g2.print(t, 1);
    u8g2.print(" C");

    u8g2.setCursor(0, 48);
    u8g2.print("Humidity: ");
    u8g2.print(h, 1);
    u8g2.print(" %");

    u8g2.setCursor(0, 64);
    u8g2.print("Press:    ");
    u8g2.print(p, 0);
    u8g2.print(" hPa");

  } else {

    u8g2.drawStr(0, 12, "SENSOR ERROR");

    u8g2.drawHLine(0, 16, 128);

    u8g2.drawStr(0, 36, "Sensor Not Found");

    u8g2.drawStr(0, 52, "Check Wires");

  }

  u8g2.sendBuffer();

  delay(2000);
}