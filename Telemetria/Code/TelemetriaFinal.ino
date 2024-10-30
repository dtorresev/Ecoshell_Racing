#include <Arduino.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <TFT_eSPI.h>  // Librería para la pantalla
#include <SPI.h>

// Definición de Pines para GPS
#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);
char datoCmd = 0;
#define NMEA 0
TinyGPSPlus gps;

// Pines para el termistor
#define termistor1 32

// Pines y variables del sensor Hall
#define sensorPin 33
const float diametroLlanta = 0.6604;  // 26"
volatile unsigned long tiempoAnterior = 0, deltaTime = 0;
const unsigned long tiempoDebounce = 80;
float rpm = 0, kmh = 0;
volatile unsigned long tiempoUltimaSenal = 0;

// Tiempo límite para recibir la señal del sensor (2 segundos)
const unsigned long tiempoLimite = 2000;

// TFT eSPI
TFT_eSPI tft = TFT_eSPI();

// Variables para almacenar información GPS
String latESP, lonESP, rpmESP, speedESP, tempESP;

// Función para obtener la temperatura desde el termistor
float temperature(int analogValue, int maxAnalog = 4095, int r0 = 10000, float A = 0.001126225487912, float B = 0.000234625984645, float C = 0.000000085504096) {
  float resistance = r0 * ((float(maxAnalog) / analogValue) - 1);
  float temperatura = A + B * log(resistance) + C * pow(log(resistance), 3);
  return (1.0 / temperatura) - 273.15;
}

// Función para el sensor Hall
void IRAM_ATTR detectarSensor() {
  unsigned long tiempoActualISR = millis();
  if ((tiempoActualISR - tiempoAnterior) > tiempoDebounce) {
    deltaTime = tiempoActualISR - tiempoAnterior;
    tiempoAnterior = tiempoActualISR;
    rpm = 60000.0 / deltaTime;
    rpmESP = String(rpm);
    tiempoUltimaSenal = tiempoActualISR;
  }
}

// Función para mostrar datos del GPS en Serial
void Visualizacion_Serial() {
  if (gps.location.isValid()) {
    latESP = String(gps.location.lat(), 10);
    lonESP = String(gps.location.lng(), 10);
  } else {
    latESP = "Sin señal GPS";
    lonESP = "Sin señal GPS";
  }
}

// Función para actualizar la pantalla TFT
void actualizarPantalla() {
  tft.fillScreen(TFT_BLACK); // Limpiar pantalla
  tft.setCursor(10, 20); 
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.setTextSize(2);

  // Imprimir datos del GPS
  tft.println("GPS:");
  tft.print("Lat: "); tft.println(latESP);
  tft.print("Lon: "); tft.println(lonESP);

  // Imprimir datos de temperatura
  tft.setCursor(10, 100);
  tft.println("Temperatura:");
  tft.print(tempESP); tft.println(" C");

  // Imprimir datos del sensor Hall
  tft.setCursor(10, 160);
  tft.println("Velocidad:");
  tft.print("RPM: "); tft.println(rpmESP);
  tft.print("KM/h: "); tft.println(speedESP);
}

void setup() {
  Serial.begin(115200);

  // Iniciar TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Inicializar GPS
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // Inicializar termistor y sensor Hall
  pinMode(termistor1, INPUT);
  pinMode(sensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(sensorPin), detectarSensor, FALLING);

  delay(2000);
}

void loop() {
  // Sensor Hall
  if ((millis() - tiempoUltimaSenal) > tiempoLimite) {
    rpm = 0;
  }
  kmh = (rpm * diametroLlanta * PI * 60) / 1000;
  speedESP = String(kmh);
  rpmESP = String(rpm);

  // Leer temperatura del termistor
  int analogValue1 = analogRead(termistor1);
  float temperatura1 = temperature(analogValue1);
  tempESP = String(temperatura1);

  // Actualizar GPS
  if (NMEA) {
    while (neogps.available()) {
      datoCmd = (char)neogps.read();
    }
  } else {
    bool newData = false;
    for (unsigned long start = millis(); millis() - start < 1000;) {
      while (neogps.available()) {
        if (gps.encode(neogps.read())) {
          newData = true;
        }
      }
    }
    if (newData) {
      Visualizacion_Serial();
    }
  }

  // Actualizar pantalla con los datos
  actualizarPantalla();
  
  delay(500); // Esperar un poco antes de la siguiente actualización
}
