#include <SPI.h>

// Pines SPI compartidos (conexión en bus)
#define SCK_PIN   18
#define MOSI_PIN  23
#define MISO_PIN  19

// Pines CS (Chip Select) para cada sensor
#define CS_SENSOR1  25
#define CS_SENSOR2  26
#define CS_SENSOR3  14

// Registros del ADXL345
#define POWER_CTL   0x2D   // Control de alimentación
#define DATA_FORMAT 0x31   // Formato de datos (rango)
#define DATAZ0      0x36   // Dirección base de datos (Z LSB) documentacion, 0x32 esta desde el lsb x

// Configuración de sensores
const int csPins[] = {CS_SENSOR1, CS_SENSOR2, CS_SENSOR3};
const int NUM_SENSORES = 3;


void setup() {
  Serial.begin(115200);
  delay(2000);

  // Inicializar SPI en modo 3, 1 MHz
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_SENSOR1);
  SPI.setDataMode(SPI_MODE3);
  SPI.setFrequency(2000000);//El sensor aguanta hasta 5mhz creo

  // Configurar pines CS como salidas y ponerlos en HIGH
  for (int i = 0; i < NUM_SENSORES; i++)
  {
    pinMode(csPins[i], OUTPUT);
    digitalWrite(csPins[i], HIGH);
  }

  // Inicializar cada sensor y verificar ID
  for (int i = 0; i < NUM_SENSORES; i++)
  {
    writeRegister(csPins[i], DATA_FORMAT, 0x0B); // Establece rango 16g
    writeRegister(csPins[i], POWER_CTL, 0x08);   // Modo medición
    delay(10);

    uint8_t id = readRegister(csPins[i], 0x00);  // DEVID debe ser 0xE5
    if (id != 0xE5)
      Serial.printf("Sensor CSPIN=%i no detectado (ID=0x%02X)\n", csPins[i], id);
  }
}

void loop() {
  // Leer los tres valores Z
  float z[]={leerZ(csPins[0]),leerZ(csPins[1]),leerZ(csPins[2])};
  
  //Gestion de problemas con el cableado, luego se puede sustituir por filtros para tomar en cuenta las mediciones
  for(int i=0; i<NUM_SENSORES; i++)  if(z[i] > 16.0 || z[i] < -16.0) z[i] = 0; 
  
  // Imprimir los tres valores separados por espacio (formato plotter)
  for(int i=0;i<NUM_SENSORES;i++)
  {
    Serial.print(z[i]);
    Serial.print(' ');
  }
  Serial.println();
  delay(50);  // Frecuencia de actualización
}

// Escribe un valor en un registro del sensor (csPin: CS del sensor)
void writeRegister(int csPin, uint8_t reg, uint8_t value)
{
  digitalWrite(csPin, LOW);//Enciende
  SPI.transfer(reg);//Indica el modo del registro como modos de energia y formato
  SPI.transfer(value);//Indica que se hace en el registro rangos o mediciones 
  digitalWrite(csPin, HIGH);//Apaga
}

// Lee un registro del sensor y devuelve su valor
uint8_t readRegister(int csPin, uint8_t reg)
{
  char address = 0x80 | reg;   // Bit de lectura (MSB=1) operacion binaria or, obtiene ultimo bit
  digitalWrite(csPin, LOW); //enciende
  SPI.transfer(address);// indicamos que vamos a leer el contenido
  uint8_t result = SPI.transfer(0x00); //Enviamos un byte dummy y recibimos el dato del sensor
  digitalWrite(csPin, HIGH); //apaga
  return result;//Devuelve resultado
}

// Lee los 6 bytes de datos (X,Y,Z) y devuelve únicamente el valor Z
float leerZ(int csPin) {
  /* Dirección de lectura multibyte: bit de lectura + bit multibyte + dirección base -----Parte interesante: de Arturo yipi
    Bit 7 (0x80): Determina si es LECTURA (1) o ESCRITURA (0)

    Bit 6 (0x40): Determina si es MODO MULTIBYTE (1) o SINGLE BYTE (0)

    Bits 5-0 (0x36): La dirección del registro donde empezar (0-63), lee todos por eso tomamos despues los primeros dos apartir de este numero
  */
  char address = 0x80 | 0x40 | DATAZ0; //Indica  que se lee
  uint8_t zLsb, zMsb;

  digitalWrite(csPin, LOW);
  SPI.transfer(address);
  zLsb = SPI.transfer(0x00); // primer byte = Z LSB //0x36
  zMsb = SPI.transfer(0x00); // segundo byte = Z MSB//0x37
  digitalWrite(csPin, HIGH);

  return (((int16_t)((zMsb << 8) | zLsb))/32.0); //Combina los bytes, desplazando el mas significativo y parseando todo a entero
}
