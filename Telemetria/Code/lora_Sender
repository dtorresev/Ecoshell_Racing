#include <LoRa.h>

// Configuración de los pines de LoRa
#define ss 5
#define rst 14
#define dio0 2

// Dirección de los dispositivos
byte LocalAddress = 0x02;               // Dirección de este dispositivo (Maestro)
byte Destination_ESP32_Slave_1 = 0x01;  // Dirección del destinatario (esclavo)

// Variables a enviar
float voltage = 12.5;
float amps = 1.2;
int temp = 25;
float rpm = 1500;
float throttle = 50.0;

// Configuración de temporización
unsigned long previousMillis_SendMSG = 0;
const long interval_SendMSG = 1000;

void setup() {
  Serial.begin(115200);

  // Inicialización de LoRa
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Error al iniciar LoRa.");
    while (true);
  }
  Serial.println("LoRa iniciado correctamente.");
}

void loop() {
  unsigned long currentMillis_SendMSG = millis();

  // Enviar datos a intervalos regulares
  if (currentMillis_SendMSG - previousMillis_SendMSG >= interval_SendMSG) {
    previousMillis_SendMSG = currentMillis_SendMSG;

    obtenerDatos();
    // Preparar los datos a enviar en formato de cadena
    String message = String(voltage) + "," + String(amps) + "," + String(temp) + "," + String(rpm) + "," + String(throttle);

    // Enviar el mensaje al esclavo
    sendMessage(message, Destination_ESP32_Slave_1);

    Serial.println("Mensaje enviado: " + message);
  }
}

void sendMessage(String outgoing, byte destination) {
  LoRa.beginPacket();
  LoRa.write(destination);        // Dirección del destinatario
  LoRa.write(LocalAddress);       // Dirección del remitente
  LoRa.write(outgoing.length());  // Longitud del mensaje
  LoRa.print(outgoing);           // Mensaje
  LoRa.endPacket();
}

void obtenerDatos() {
  rpm = random(0, 201);

  temp = random(0, 101);

  voltage = random(0, 37);

  amps = random(0, 8);

  throttle = random(0, 101);
}


// Quitar obtenerDatos y reemplazar por la obtención real de los datos de los sensores, igualmente añadir las variables requeridas y datos que quieran mandar al
// string "message"
