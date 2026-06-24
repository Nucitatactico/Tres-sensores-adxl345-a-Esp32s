#include <SPI.h>

#define CS_PIN 26
#define SCK_PIN 18
#define MOSI_PIN 23
#define MISO_PIN 19

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("Probando ADXL345 con CS en GPIO25");
  
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  
  // Intentar leer Device ID (debería ser 0xE5)
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x80 | 0x00); // Leer DEVID (0x00)
  uint8_t id = SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  
  Serial.printf("Device ID: 0x%02X (debería ser 0xE5)\n", id);
  
  if(id == 0xE5) {
    Serial.println("Sensor detectado con CS!");
  } else {
    Serial.println("snsor NO detectado. Verifica conexiones.");
    Serial.println("   - GPIO18 -> SCL");
    Serial.println("   - GPIO19 -> SDO (MISO)");
    Serial.println("   - GPIO23 -> SDA (MOSI)");
    Serial.println("   - GPIO25 -> CS");
    Serial.println("   - 3.3V y GND");
  }
}

void loop() {}
