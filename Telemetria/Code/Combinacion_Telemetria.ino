#include <Arduino.h>
#include <Wire.h>
#include <TinyGPS++.h>

#include <WiFi.h>
#include <HTTPClient.h>

//Definición de Pines para GPS y iniciación de GPS
#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);
char datoCmd = 0;
#define NMEA 0
TinyGPSPlus gps;

//Definión de pines para Termistor
#define termistor1 35

//Definición de pines y variables de Sensor Hall

#define sensorPin 33
const float diametroLlanta = 0.6604;  // 26"
volatile unsigned long tiempoAnterior = 0, tiempoActual = 0, deltaTime = 0;
const unsigned long tiempoDebounce = 80;
float rpm = 0, kmh = 0;

// Tiempo límite para recibir la señal del sensor (2 segundos)
const unsigned long tiempoLimite = 2000;
volatile unsigned long tiempoUltimaSenal = 0;

// Variables para almacenar los valores anteriores
float rpmAnterior = -1, kmhAnterior = -1;

// URL del archivo php que manda los datos a la DB  (Se debe de editar la IP si se cambia de dispositivo o red, "ipconfig" en la cmd para revisar)
String URL = "http://172.20.10.3/ecoshellDB/ecoShell.php";

// Constantes de los datos de la red utilizada
const char* ssid = "Juanjo"; // nombre de la red
const char* password = "ecoshell"; // contraseña de la red

// Variables a mandar al codigo php
String rpmESP = ""; // ok
String speedESP = ""; // ok
String tempESP = ""; // ok
String voltESP = "";
String ampESP = "";
String throtESP = "";
String latESP = ""; // ok
String lonESP = ""; // ok

//------------------------------------------------------------------------------------------------------------------------------------------

//Funciones para sensor GPS

void Visualizacion_Serial(void)
{ 
   
  if (gps.location.isValid() ==  1)
  {  
    //Serial.print("Lat: ");
    latESP = String(gps.location.lat(),10);
    Serial.println(latESP);
    //Serial.print("Lng: ");
    lonESP = String(gps.location.lng(),10);
    Serial.println(lonESP);  
  }
  else
  {
    Serial.println("Sin señal gps");  
  }  

}

//Funciones para sensor de Temperatura

float temperature(int analogValue, int maxAnalog = 4095, int r0 = 10000, float A = 0.001126225487912, float B = 0.000234625984645, float C = 0.000000085504096){
  float resistance = r0 * ((float(maxAnalog)/ analogValue) - 1);
  float temperatura = A + B * log(resistance) + C * pow(log(resistance),3);
  return (1.0/temperatura) - 273.15;
}

//Funciones para el Sensor HALL

void IRAM_ATTR detectarSensor() {
  unsigned long tiempoActualISR = millis();
  if ((tiempoActualISR - tiempoAnterior) > tiempoDebounce) {
    deltaTime = tiempoActualISR - tiempoAnterior;
    tiempoAnterior = tiempoActualISR;
    rpm = 60000.0 / deltaTime;
    rpmESP = String(rpm);
    // Actualizar el tiempo de la última señal recibida
    tiempoUltimaSenal = tiempoActualISR;
  }
}

// Funciones para Database

void connectWifi() {
  WiFi.begin(ssid, password);
  Serial.println("Conectando a WiFi");

  int max_attempts = 20;
  int attempt = 0;

  while (WiFi.status() != WL_CONNECTED && attempt < max_attempts) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Conectado a: ");
    Serial.println(ssid);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("No se pudo conectar a WiFi");
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void setup() 
{
  Serial.begin(115200);

  //Definición de inicio de GPS
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);

  //Definición de inicio de Termistor
  pinMode(termistor1, INPUT);

  //Definición de inicio del Sensor Hall
  pinMode(sensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(sensorPin), detectarSensor, FALLING);

   // Inicializar la conexion WiFi
  connectWifi();
  
  delay(2000);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void loop() 
{

//-------------- Codigo Hall Sensor --------------

if ((millis() - tiempoUltimaSenal) > tiempoLimite) {
    rpm = 0;
}

kmh = (rpm * diametroLlanta * PI * 60) / 1000;
speedESP = String(kmh);
   
if (rpm != rpmAnterior) {
    rpmAnterior = rpm;
    Serial.println(rpm);
}

if (kmh != kmhAnterior) {
    kmhAnterior = kmh;
    Serial.println(kmh);
}

//-------------- Codigo Termistor --------------

// Leer los valores analógicos de los pines
int analogValue1 = analogRead(termistor1);
//int analogValue2 = analogRead(termistor2);
//int analogValue3 = analogRead(termistor3);

// Convertir los valores analógicos a temperatua
float temperatura1 = temperature(analogValue1);
tempESP = String(temperatura1);
//float temperature2 = temperature(analogValue2);
//float temperature3 = temperature(analogValue3);

// Imprimir los valores de temperatura
Serial.print("Temperatura en termistor 1 -> ");
Serial.print(temperatura1);
Serial.println("°C");

//Serial.print("Temperatura en termistor 2 -> ");
//Serial.print(temperatura2);
//Serial.println("°C");

//Serial.print("Temperatura en termistor 3 -> ");
//Serial.print(temperatura3);
//Serial.println("°C");
    
// Esperar un poco antes de la siguiente lectura
delay(500);

//-------------- Codigo GPS --------------

if (NMEA){
    while (neogps.available()){
     datoCmd  = (char)neogps.read(); 
     //Serial.print(datoCmd );
    }
} else {
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;) {
    while (neogps.available()) {
      if (gps.encode(neogps.read())) {
        newData = true;         
      }
    }
  }  
  if(newData == true){
    newData = false;
    //Serial.println(gps.satellites.value());    
    Visualizacion_Serial();
  } 
 }
   
//-------------- Codigo Database --------------
   
// Si la ESP se desconecta, volver a hacerlo
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
   
  // Datos a enviar al codigo de php
  String postData = "rpmESP=" + rpmESP + "speedESP=" + speedESP + "&tempESP=" + tempESP + "&voltESP=" + voltESP + "&ampESP=" + ampESP + "&throtESP=" + throtESP + "&latESP=" + latESP + "&lonESP=" + lonESP;

  // Inicializar la conexion con el codigo php
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(URL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Enviar el string de datos
    int httpCode = http.POST(postData);

    // Respuesta del archivo php (Es lo mismo que manda a imprimir el php al recibir o no los datos)
    String respuesta = http.getString();

    Serial.print("Respuesta: ");
    Serial.println(respuesta);
    Serial.println("-----------------------------------");

    http.end();  // Finaliza la conexión
  }
}
