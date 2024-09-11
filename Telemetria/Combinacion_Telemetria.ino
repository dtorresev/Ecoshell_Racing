#include <Arduino.h>
#include <Wire.h>
#include <TinyGPS++.h>

//Definición de Pines para GPS y iniciación de GPS
#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);
char   datoCmd = 0;
#define NMEA 0
TinyGPSPlus gps;

//Definión de pines para Termistor
#define termistor1 35

//Definición de pines y variables de Sensor Hall

#define sensorPin 34;
const float diametroLlanta = 0.6604;  // 26"
volatile unsigned long tiempoAnterior = 0, tiempoActual = 0, deltaTime = 0;
const unsigned long tiempoDebounce = 80;
float rpm = 0, kmh = 0;

// Tiempo límite para recibir la señal del sensor (2 segundos)
const unsigned long tiempoLimite = 2000;
volatile unsigned long tiempoUltimaSenal = 0;

// Variables para almacenar los valores anteriores
float rpmAnterior = -1, kmhAnterior = -1;

//------------------------------------------------------------------------------------------------------------------------------------------

//Funciones para sensor GPS

void Visualizacion_Serial(void)
{ 
   
  if (gps.location.isValid() ==  1)
  {  
    //Serial.print("Lat: ");
    Serial.println(gps.location.lat(),10);
    //Serial.print("Lng: ");
    Serial.println(gps.location.lng(),10);  
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
    // Actualizar el tiempo de la última señal recibida
    tiempoUltimaSenal = tiempoActualISR;
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
delay(1000);

//-------------- Codigo GPS --------------

if (NMEA)
 {
    while (neogps.available())
    {
     datoCmd  = (char)neogps.read(); 
     //Serial.print(datoCmd );
    }
 } 

else
 {
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (neogps.available())
    {
      if (gps.encode(neogps.read()))
      {
        newData = true;         
      }
    }
  }  

  if(newData == true)
  {
    newData = false;
    //Serial.println(gps.satellites.value());    
    Visualizacion_Serial();
  }
  else
  {
    
  }  
  
 }
}