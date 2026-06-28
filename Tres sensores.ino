#include <SPI.h>

// Pines SPI
#define SCK_PIN   18
#define MOSI_PIN  23
#define MISO_PIN  19

// Pines CS para los 3 sensores
const int csPins[3] = {25, 26, 27};
const int N_SENSORES = 3;

// Registros del ADXL345
#define POWER_CTL   0x2D
#define DATA_FORMAT 0x31
#define DATAX0      0x32

// Variables para almacenar si cada sensor fue detectado
bool sensoresOK[N_SENSORES];

// Prototipos
void writeRegister(int csPin, char reg, char value);
uint8_t readRegister(int csPin, char reg);
void leerAceleracion(int csPin, int16_t &x, int16_t &y, int16_t &z);

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  // Inicializar bus SPI
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, -1);  // -1 = no CS fijo
  SPI.setDataMode(SPI_MODE3);
  SPI.setFrequency(4000000);  // 1 MHz es seguro
  
  // Configurar todos los pines CS como salida y ponerlos en HIGH (inactivo)
  for (int i = 0; i < N_SENSORES; i++) {
    pinMode(csPins[i], OUTPUT);
    digitalWrite(csPins[i], HIGH);
  }
  
  // Inicializar cada sensor
  for (int i = 0; i < N_SENSORES; i++)
  {
    Serial.printf("Inicializando Sensor %d (GPIO%d)...\n", i+1, csPins[i]);
    
    // Configurar ±16g y modo medición
    writeRegister(csPins[i], DATA_FORMAT, 0x0B);
    writeRegister(csPins[i], POWER_CTL, 0x08);
    delay(10);
    
    // Verificar ID (DEVID debe ser 0xE5)
    uint8_t id = readRegister(csPins[i], 0x00);
    sensoresOK[i] = (id == 0xE5);
    Serial.printf("Sensor %d ID: 0x%02X %s\n", i+1, id, sensoresOK[i] ? "✅" : "❌");
  }
  
  // Resumen final
  int okCount = 0;
  for (int i = 0; i < N_SENSORES; i++) {
    if (sensoresOK[i]) okCount++;
  }
  if (okCount == N_SENSORES) {
    Serial.println("Sensores detectados");
  } else {
    Serial.printf("Solo %d de %d sensores fueron detectados.\n", okCount, N_SENSORES);
  }
}

void loop() {
  // Leer solo si todos los sensores están OK (opcional)
  bool todosOK = true;
  for (int i = 0; i < N_SENSORES; i++) {
    if (!sensoresOK[i]) { todosOK = false; break; }
  }
  
  if (todosOK) {
    int16_t x, y, z;
    
    for (int i = 0; i < N_SENSORES; i++) {
      leerAceleracion(csPins[i], x, y, z);
      Serial.printf("S%d: X=%5d Y=%5d Z=%5d  ", i+1, x, y, z);
    }
    Serial.println();
    delay(100);
  } else {
    // Si algún sensor falló, reintentar inicialización o simplemente esperar
    Serial.println("⏳ Revisando sensores...");
    delay(2000);
    // Podríamos reintentar la inicialización aquí, pero por simplicidad solo esperamos.
  }
}

// ===== Funciones auxiliares (igual que tu código) =====

void writeRegister(int csPin, char reg, char value) {
  digitalWrite(csPin, LOW);
  SPI.transfer(reg);
  SPI.transfer(value);
  digitalWrite(csPin, HIGH);
  delay(1);
}

uint8_t readRegister(int csPin, char reg) {
  char address = 0x80 | reg;  // Bit de lectura
  digitalWrite(csPin, LOW);
  SPI.transfer(address);
  uint8_t result = SPI.transfer(0x00);
  digitalWrite(csPin, HIGH);
  delay(1);
  return result;
}

void leerAceleracion(int csPin, int16_t &x, int16_t &y, int16_t &z) {
  unsigned char values[6];
  char address = 0x80 | 0x40 | DATAX0;  // Lectura + multibyte
  
  digitalWrite(csPin, LOW);
  SPI.transfer(address);
  
  for (int i = 0; i < 6; i++) {
    values[i] = SPI.transfer(0x00);
  }
  
  digitalWrite(csPin, HIGH);
  
  // Combinar bytes (little-endian)
  x = (int16_t)((values[1] << 8) | values[0]);
  y = (int16_t)((values[3] << 8) | values[2]);
  z = (int16_t)((values[5] << 8) | values[4]);
}
