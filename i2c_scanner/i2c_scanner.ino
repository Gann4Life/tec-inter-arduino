#include <Wire.h>

// Scans the I2C bus and prints any addresses found.
// Open Serial Monitor at 9600 baud after uploading.
// Use this if the LCD stays blank: it tells you the real address (0x27 or 0x3F).

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("I2C scanner started");
}

void loop() {
  byte count = 0;

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      count++;
    }
  }

  if (count == 0) {
    Serial.println("No I2C devices found - check SDA/SCL wiring and power");
  }

  Serial.println("---");
  delay(3000);
}
