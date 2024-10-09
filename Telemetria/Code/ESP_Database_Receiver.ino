#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <LoRa.h>

// URL del archivo PHP que manda los datos a la DB (cambia la IP según la red)
String URL = "http://172.20.10.3/ecoshellDB/ecoShell.php";

// Credenciales de Wi-Fi
const char* ssid = "Juanjo";
const char* password = "ecoshell";

// Variables a mandar al código PHP
String rpmESP = "";
String speedESP = "";
String tempESP = "";
String voltESP = "";
String ampESP = "";
String throtESP = "";
String latESP = "";
String lonESP = "";

// Pines de LoRa
#define ss 5
#define rst 14
#define dio0 2

// Configuración de direcciones LoRa
byte LocalAddress = 0x01;               // Dirección de este dispositivo (Maestro)
byte Destination_ESP32_Slave_1 = 0x02;  // Dirección de destino (esclavo 1)

// Estructura para recibir datos
struct dataPack {
  float voltage; 
  int temp;
  float speed;
  float amps; 
  float rpm; 
  float throttle; 
};

dataPack receiveData;

// Contador para reiniciar LoRa
unsigned long previousMillis_RestartLORA = 0;
const long interval_RestartLORA = 1000;
byte Count_to_Rst_LORA = 0;

// Variables para depuración
String Incoming = "";

// Función para conectarse a Wi-Fi
void connectWifi() {
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int max_attempts = 20;
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < max_attempts) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("Conectado a WiFi.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("No se pudo conectar a WiFi.");
  }
}

// Reiniciar LoRa
void Rst_LORA() {
  LoRa.setPins(ss, rst, dio0);
  Serial.println("Reiniciando LoRa...");
  
  if (!LoRa.begin(433E6)) {  // Inicializar LoRa a 433 MHz
    Serial.println("Fallo al iniciar LoRa. Revisa conexiones.");
    while (true);
  }
  Serial.println("LoRa iniciado correctamente.");
  
  Count_to_Rst_LORA = 0;  // Reiniciar el contador de LoRa
}

// Recibir datos vía LoRa
void onReceive(int packetSize) {
  if (packetSize == 0) return;  // Si no hay paquetes, salir

  Serial.println("Recibiendo paquete LoRa...");

  int recipient = LoRa.read();        // Dirección del destinatario
  byte sender = LoRa.read();          // Dirección del remitente
  byte incomingLength = LoRa.read();  // Longitud del mensaje recibido

  Incoming = "";  // Limpiar el buffer de datos recibidos
  while (LoRa.available()) {
    Incoming += (char)LoRa.read();  // Leer los datos entrantes
  }

  Serial.println("Mensaje recibido: " + Incoming);

  // Comprobar si la longitud coincide
  if (incomingLength != Incoming.length()) {
    Serial.println("Error: la longitud del mensaje no coincide.");
    return;
  }

  // Comprobar si el mensaje es para este dispositivo
  if (recipient != LocalAddress) {
    Serial.println("Este mensaje no es para mí.");
    return;
  }

  // Procesar los datos recibidos
  Processing_incoming_data(sender);
}

// Procesar los datos recibidos
void Processing_incoming_data(byte sender) {
  if (sender == Destination_ESP32_Slave_1) {
    // Parsear los datos
    receiveData.voltage = GetValue(Incoming, ',', 0).toFloat();
    receiveData.amps = GetValue(Incoming, ',', 1).toFloat();
    receiveData.temp = GetValue(Incoming, ',', 2).toInt();
    receiveData.rpm = GetValue(Incoming, ',', 3).toFloat();
    receiveData.throttle = GetValue(Incoming, ',', 4).toFloat();

    // Asignar datos a variables para enviar
    rpmESP = String(receiveData.rpm);
    speedESP = String((receiveData.rpm * 0.6604 * PI * 60) / 1000);  // Cálculo de la velocidad
    tempESP = String(receiveData.temp);
    voltESP = String(receiveData.voltage);
    ampESP = String(receiveData.amps);
    throtESP = String(receiveData.throttle);
  }
}

// Obtener un valor de una cadena de texto separada por comas
String GetValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setup() {
  Serial.begin(115200);

  // Conectar a Wi-Fi
  connectWifi();
  
  // Inicializar LoRa
  Rst_LORA();
}

void loop() {
  unsigned long currentMillis_RestartLORA = millis();
  
  // Reiniciar LoRa si no hay comunicación durante un tiempo
  if (currentMillis_RestartLORA - previousMillis_RestartLORA >= interval_RestartLORA) {
    previousMillis_RestartLORA = currentMillis_RestartLORA;
    Count_to_Rst_LORA++;
    
    if (Count_to_Rst_LORA > 30) {
      LoRa.end();
      Rst_LORA();
    }
  }

  // Parsear los paquetes entrantes de LoRa
  onReceive(LoRa.parsePacket());

  // Reconectar a Wi-Fi si se pierde la conexión
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }

  Serial.println("Mensaje recibido: " + Incoming);

  // Enviar datos al servidor PHP
  String postData = "rpmESP=" + rpmESP + "&speedESP=" + speedESP + "&tempESP=" + tempESP + "&voltESP=" + voltESP + "&ampESP=" + ampESP + "&throtESP=" + throtESP + "&latESP=" + latESP + "&lonESP=" + lonESP;
  //Serial.println("Datos a enviar: " + postData);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(URL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpCode = http.POST(postData);
    String respuesta = http.getString();

    Serial.print("Respuesta del servidor: ");
    Serial.println(respuesta);

    http.end();
  } else {
    //Serial.println("WiFi no conectado. Intentando reconectar...");
  }
}
