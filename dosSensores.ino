#include <SPI.h>

// Definir pines SPI
#define SCK_PIN   18
#define MOSI_PIN  23
#define MISO_PIN  19

// Definir pines CS para cada sensor
#define CS_SENSOR1  26
#define CS_SENSOR2  27

// Registros del ADXL345
#define POWER_CTL   0x2D
#define DATA_FORMAT 0x31
#define DATAX0      0x32

unsigned long tiempo;

void setup() {
  Serial.begin(115200);
  delay(2000);
  // Inicializar SPI
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_SENSOR1);
  SPI.setDataMode(SPI_MODE3);
  SPI.setFrequency(1000000);
  
  // Configurar pines CS
  pinMode(CS_SENSOR1, OUTPUT);
  pinMode(CS_SENSOR2, OUTPUT);
  digitalWrite(CS_SENSOR1, HIGH);
  digitalWrite(CS_SENSOR2, HIGH);
  
  // Inicializar Sensor 1
  Serial.println("Inicializando Sensor 1 (GPIO26)...");
  writeRegister(CS_SENSOR1, DATA_FORMAT, 0x0B);  // ±16g
  writeRegister(CS_SENSOR1, POWER_CTL, 0x08);    // Modo medición
  delay(10);
  
  // Verificar Sensor 1
  uint8_t id1 = readRegister(CS_SENSOR1, 0x00);  // DEVID
  Serial.printf("Sensor 1 ID: 0x%02X %s\n", id1, (id1 == 0xE5) ? "✅" : "❌");
  
  // Inicializar Sensor 2
  Serial.println("Inicializando Sensor 2 (GPIO27)...");
  writeRegister(CS_SENSOR2, DATA_FORMAT, 0x0B);  // ±16g
  writeRegister(CS_SENSOR2, POWER_CTL, 0x08);    // Modo medición
  delay(10);
  
  // Verificar Sensor 2
  uint8_t id2 = readRegister(CS_SENSOR2, 0x00);  // DEVID
  Serial.printf("Sensor 2 ID: 0x%02X %s\n", id2, (id2 == 0xE5) ? "✅" : "❌");
  
  Serial.println("=========================================");
  
  if(id1 != 0xE5 || id2 != 0xE5) {
    Serial.println("⚠️  Algunos sensores no fueron detectados");
    Serial.println("   Verifica las conexiones:");
    Serial.println("   - GPIO18 -> SCL (ambos)");
    Serial.println("   - GPIO23 -> SDA (ambos)");
    Serial.println("   - GPIO19 -> SDO (ambos)");
    Serial.println("   - GPIO26 -> CS (Sensor 1)");
    Serial.println("   - GPIO27 -> CS (Sensor 2)");
  } else {
    Serial.println("🎉 ¡Ambos sensores detectados correctamente!");
  }
}

void loop() {
  // Solo leer si ambos sensores fueron detectados
  if(readRegister(CS_SENSOR1, 0x00) == 0xE5 && readRegister(CS_SENSOR2, 0x00) == 0xE5) {
    
    // Leer Sensor 1
    int16_t x1, y1, z1;
    leerAceleracion(CS_SENSOR1, x1, y1, z1);
    
    // Leer Sensor 2
    int16_t x2, y2, z2;
    leerAceleracion(CS_SENSOR2, x2, y2, z2);
    
    // Mostrar resultados
    Serial.printf("S1: X=%d Y=%d Z=%d | S2: X=%d Y=%d Z=%d\n", 
                  x1, y1, z1, x2, y2, z2);
    delay(100);
  } else {
    Serial.println("⏳ Esperando sensores...");
    delay(2000);
  }
}

// Función para escribir en un registro
void writeRegister(int csPin, char reg, char value) {
  digitalWrite(csPin, LOW);
  SPI.transfer(reg);
  SPI.transfer(value);
  digitalWrite(csPin, HIGH);
  delay(1);
}

// Función para leer un registro
uint8_t readRegister(int csPin, char reg) {
  char address = 0x80 | reg;  // Bit de lectura
  digitalWrite(csPin, LOW);
  SPI.transfer(address);
  uint8_t result = SPI.transfer(0x00);
  digitalWrite(csPin, HIGH);
  delay(1);
  return result;
}

// Función para leer aceleración
void leerAceleracion(int csPin, int16_t &x, int16_t &y, int16_t &z) {
  unsigned char values[6];
  char address = 0x80 | 0x40 | DATAX0;  // Lectura + multibyte
  
  digitalWrite(csPin, LOW);
  SPI.transfer(address);
  
  for(int i = 0; i < 6; i++) {
    values[i] = SPI.transfer(0x00);
  }
  
  digitalWrite(csPin, HIGH);
  
  // Combinar bytes (little-endian)
  x = (int16_t)((values[1] << 8) | values[0]);
  y = (int16_t)((values[3] << 8) | values[2]);
  z = (int16_t)((values[5] << 8) | values[4]);
}
