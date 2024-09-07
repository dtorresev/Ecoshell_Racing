#include <Arduino.h>

// Pines donde se conectarán los sensores analógicos
const int termistor1 = A0;
//const int termistor1 = 32; 
//const int termistor2 = 35;
//const int termistor3 = 34;

float temperature(int analogValue, int maxAnalog = 1023, int r0 = 10000, float A = 0.001126225487912, float B = 0.000234625984645, float C = 0.000000085504096){
  float resistance = r0 * ((float(maxAnalog)/ analogValue) - 1);
  float temperatura = A + B * log(resistance) + C * pow(log(resistance),3);
  return (1.0/temperatura) - 273.15;
}

void setup() {
    // Inicializar la comunicación serie
    Serial.begin(9600);
    //Serial.begin(115200);
    // Configurar los pines analógicos como entrada
    pinMode(termistor1, INPUT);
    //pinMode(termistor2, INPUT);
    //pinMode(termistor3, INPUT);
}

void loop() {
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
}
