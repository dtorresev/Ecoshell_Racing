#include <Arduino.h>
#include <TFT_eSPI.h>

// Configuración de pines para la pantalla TFT
#define TFT_CS 15
#define TFT_RST 4
#define TFT_DC 2
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_MISO 19
TFT_eSPI tft = TFT_eSPI();

const int sensorPin = 34;
const float diametroLlanta = 0.6604;  // 26"
volatile unsigned long tiempoAnterior = 0, tiempoActual = 0, deltaTime = 0;
const unsigned long tiempoDebounce = 80;
float rpm = 0, kmh = 0;

// Tiempo límite para recibir la señal del sensor (2 segundos)
const unsigned long tiempoLimite = 2000;
volatile unsigned long tiempoUltimaSenal = 0;

// Variables para almacenar los valores anteriores
float rpmAnterior = -1, kmhAnterior = -1;

void IRAM_ATTR detectarSensor() {
  unsigned long tiempoActualISR = millis();
  if ((tiempoActualISR - tiempoAnterior) > tiempoDebounce) {
    deltaTime = tiempoActualISR - tiempoAnterior;
    tiempoAnterior = tiempoActualISR;
    rpm = 60000.0 / deltaTime;
    // Actualizar el tiempo de la última señal recibida
    tiempoUltimaSenal = tiempoActualISR;
  }
}

void setup() {
  pinMode(sensorPin, INPUT);
  //Serial.begin(115200);
  
  tft.begin();
  tft.setRotation(1);         // Ajustar la rotación según sea necesario
  tft.fillScreen(TFT_BLACK);  // Limpiar la pantalla al inicio
  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE);
  
  // Imprimir las etiquetas fijas
  tft.setCursor(10, 10);
  tft.print("RPM: ");
  tft.setCursor(10, 50);
  tft.print("KMH: ");

  // Dibujar barra de velocidad inicial
  tft.drawRect(10, 90, tft.width() - 20, 40, TFT_WHITE);
  
  // Configurar la interrupción externa para el pin del sensor
  attachInterrupt(digitalPinToInterrupt(sensorPin), detectarSensor, FALLING);
}

void loop() {
  // Comprobar si ha pasado más de 2 segundos desde la última señal
  if ((millis() - tiempoUltimaSenal) > tiempoLimite) {
    rpm = 0;
  }
  
  // Calcular la velocidad en km/h
  kmh = (rpm * diametroLlanta * PI * 60) / 1000;
  
  /*
  // Imprimir los valores en el monitor serie
  Serial.print("RPM: ");
  Serial.println(rpm);
  Serial.print("KMH: ");
  Serial.println(kmh);
*/

  // Actualizar la pantalla solo si los valores han cambiado
  if (rpm != rpmAnterior) {
    tft.fillRect(100, 10, 140, 40, TFT_BLACK);  // Limpiar solo el área del texto numérico
    tft.setCursor(100, 10);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Establecer el color de fondo para limpiar automáticamente
    tft.print(rpm);
    rpmAnterior = rpm;
  }

  if (kmh != kmhAnterior) {
    tft.fillRect(100, 50, 140, 40, TFT_BLACK);  // Limpiar solo el área del texto numérico
    tft.setCursor(100, 50);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Establecer el color de fondo para limpiar automáticamente
    tft.print(kmh);
    kmhAnterior = kmh;

    // Calcular el ancho de la barra de velocidad
    int anchoBarra = map(kmh, 0, 30, 0, tft.width() - 20); // Ajustar el valor máximo según la velocidad máxima esperada
    
    // Limpiar la barra anterior
    tft.fillRect(11, 91, tft.width() - 22, 38, TFT_BLACK); 
    // Dibujar la nueva barra de velocidad
    if (kmh <= 30) {
      tft.fillRect(11, 91, anchoBarra, 38, TFT_GREEN);
    } else {
      tft.fillRect(11, 91, tft.width() - 22, 38, TFT_RED);
    }

    // Mostrar mensaje si la velocidad supera los 30 km/h
    if (kmh > 30) {
      tft.setCursor(10, 140);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.print("Te estas     mamando!");
    } else {
      // Limpiar el mensaje si la velocidad es menor o igual a 30 km/h
      tft.fillRect(10, 140, tft.width(), 60, TFT_BLACK);
    }
  }

  delay(50);
}