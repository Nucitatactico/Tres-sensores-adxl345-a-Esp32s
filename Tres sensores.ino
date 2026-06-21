#include <SPI.h>

// Definir pines SPI compartidos
#define SCK_PIN   18
#define MOSI_PIN  23
#define MISO_PIN  19

// Definir pines CS para cada sensor
#define CS_SENSOR1  26   // Sensor 1
#define CS_SENSOR2  27   // Sensor 2
#define CS_SENSOR3  25   // Sensor 3 (elige un pin libre)

// Registros del ADXL345
#define POWER_CTL   0x2D
#define DATA_FORMAT 0x31
#define DATAX0      0x32

// Array con todos los pines CS para facilitar el código
int csPins[] = {CS_SENSOR1, CS_SENSOR2, CS_SENSOR3};
const char* sensorNames[] = {"Sensor 1 (GPIO26)", "Sensor 2 (GPIO27)", "Sensor 3 (GPIO25)"};
const int NUM_SENSORES = 3;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=========================================");
  Serial.println("   PRUEBA DE 3 SENSORES ADXL345");
  Serial.println("=========================================");
  
  // Inicializar SPI
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_SENSOR1);
  SPI.setDataMode(SPI_MODE3);
  SPI.setFrequency(1000000);
  
  // Configurar todos los pines CS como salidas y ponerlos en HIGH
  for(int i = 0; i < NUM_SENSORES; i++) {
    pinMode(csPins[i], OUTPUT);
    digitalWrite(csPins[i], HIGH);
  }
  
  // Inicializar y verificar cada sensor
  int sensoresDetectados = 0;
  
  for(int i = 0; i < NUM_SENSORES; i++) {
    Serial.printf("Inicializando %s...\n", sensorNames[i]);
    
    // Configurar el sensor
    writeRegister(csPins[i], DATA_FORMAT, 0x0B);  // ±16g
    writeRegister(csPins[i], POWER_CTL, 0x08);    // Modo medición
    delay(10);
    
    // Verificar el sensor (leer Device ID)
    uint8_t id = readRegister(csPins[i], 0x00);   // DEVID
    Serial.printf("  ID: 0x%02X %s\n", id, (id == 0xE5) ? "✅" : "❌");
    
    if(id == 0xE5) sensoresDetectados++;
  }
  
  Serial.println("=========================================");
  Serial.printf("📊 Resultado: %d de %d sensores detectados\n", sensoresDetectados, NUM_SENSORES);
  
  if(sensoresDetectados == NUM_SENSORES) {
    Serial.println("🎉 ¡TODOS LOS SENSORES FUNCIONAN CORRECTAMENTE!");
  } else {
    Serial.println("⚠️  Algunos sensores no fueron detectados");
    Serial.println("   Verifica las conexiones:");
    Serial.println("   - GPIO18 -> SCL (todos)");
    Serial.println("   - GPIO23 -> SDA (todos)");
    Serial.println("   - GPIO19 -> SDO (todos)");
    Serial.println("   - GPIO26 -> CS (Sensor 1)");
    Serial.println("   - GPIO27 -> CS (Sensor 2)");
    Serial.println("   - GPIO25 -> CS (Sensor 3)");
  }
  Serial.println("=========================================");
}

void loop() {
  // Verificar que todos los sensores estén presentes
  bool todosDetectados = true;
  for(int i = 0; i < NUM_SENSORES; i++) {
    if(readRegister(csPins[i], 0x00) != 0xE5) {
      todosDetectados = false;
      break;
    }
  }
  
  if(todosDetectados) {
    // Arrays para almacenar datos de todos los sensores
    int16_t x[NUM_SENSORES], y[NUM_SENSORES], z[NUM_SENSORES];
    
    // Leer todos los sensores
    for(int i = 0; i < NUM_SENSORES; i++) {
      leerAceleracion(csPins[i], x[i], y[i], z[i]);
    }
    
    // Mostrar resultados en una sola línea
    for(int i = 0; i < NUM_SENSORES; i++) {
      Serial.printf("S%d: X=%d Y=%d Z=%d", i+1, x[i], y[i], z[i]);
      if(i < NUM_SENSORES - 1) Serial.print(" | ");
    }
    Serial.println();
    
    delay(100);
  } else {
    Serial.println("⏳ Esperando sensores... (presiona RESET para re-escanear)");
    delay(2000);
  }
}

// ========== FUNCIONES PARA TODOS LOS SENSORES ==========

// Función para escribir en un registro (recibe el pin CS como parámetro)
void writeRegister(int csPin, char reg, char value) {
  digitalWrite(csPin, LOW);
  SPI.transfer(reg);
  SPI.transfer(value);
  digitalWrite(csPin, HIGH);
  delay(1);
}

// Función para leer un registro (recibe el pin CS como parámetro)
uint8_t readRegister(int csPin, char reg) {
  char address = 0x80 | reg;  // Bit de lectura
  digitalWrite(csPin, LOW);
  SPI.transfer(address);
  uint8_t result = SPI.transfer(0x00);
  digitalWrite(csPin, HIGH);
  delay(1);
  return result;
}

// Función para leer aceleración (recibe el pin CS como parámetro)
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
