#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include <U8g2lib.h>

Adafruit_BME280 bme; 

U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8g2(U8G2_R0, 12, 11, 10, 13, 14);

int sensorType = 0; // 1 = BME280,  0 = None

void setup() {
  Serial.begin(115200);

  // Set up GPIO 6 and GPIO 7 as virtual 3.3V and GND for CS and ADDR
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); // CS = HIGH -> Forces I2C Mode

  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);  // ADDR = LOW -> Sets Address 0x76
  
  delay(100);

  Wire.begin(8, 9); // SDA=8, SCL=9

  // 1. Try BME280 at 0x76 & 0x77
  if (bme.begin(0x76, &Wire) || bme.begin(0x77, &Wire)) {
    sensorType = 1;
    Serial.println("SUCCESS: Found BME280!");
  } 
  else {
    Serial.println("ERROR: No sensor detected.");
  }
  u8g2.begin();
}

void loop() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  if (sensorType == 1) { // BME280
    float t = bme.readTemperature();
    float h = bme.readHumidity();
    float p = bme.readPressure() / 100.0F;

    u8g2.drawStr(0, 12, "BME280 MONITOR");
    u8g2.drawHLine(0, 16, 128);
    u8g2.setCursor(0, 32); u8g2.print("Temp:     " + String(t, 1) + " C");
    u8g2.setCursor(0, 48); u8g2.print("Humidity: " + String(h, 1) + " %");
    u8g2.setCursor(0, 64); u8g2.print("Press:    " + String(p, 0) + " hPa");
  } 
  else {
    u8g2.drawStr(0, 12, "SENSOR ERROR");
    u8g2.drawHLine(0, 16, 128);
    u8g2.drawStr(0, 36, "Sensor Not Found");
    u8g2.drawStr(0, 52, "Check Wires");
  }
  u8g2.sendBuffer();
  delay(2000);
}