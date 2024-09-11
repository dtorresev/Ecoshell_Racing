#include <WiFi.h>
#include <HTTPClient.h>

// URL del archivo php que manda los datos a la DB  (Se debe de editar la IP si se cambia de dispositivo o red, "ipconfig" en la cmd para revisar)
String URL = "http://172.20.10.3/ecoshellDB/ecoShell.php";

// Constantes de los datos de la red utilizada
const char* ssid = "Juanjo"; // nombre de la red
const char* password = "ecoshell"; // contraseña de la red

// Variables a mandar al codigo php
String rpmESP = "";
String tempESP = "";
String voltESP = "";
String ampESP = "";
String throtESP = "";
String latESP = "";
String lonESP = "";

// Variables a mandar al codigo php
float rpmNum = 0;
float tempNum = 0;
float voltNum = 0;
float ampNum = 0;
float throtNum = 0;
float latNum = 0;
float lonNum = 0;

void setup() {
  Serial.begin(115200);

  // Inicializar la conexion WiFi
  connectWifi();
}

void loop() {
  // Si la ESP se desconecta, volver a hacerlo
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }

  obtenerDatos();

  // Datos a enviar al codigo de php
  String postData = "rpmESP=" + rpmESP + "&tempESP=" + tempESP + "&voltESP=" + voltESP + "&ampESP=" + ampESP + "&throtESP=" + throtESP + "&latESP=" + latESP + "&lonESP=" + lonESP;

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

// Esta es la función "Padre" de la recolección de datos, si añaden una función para la obtención de su dato que se junte en esta y quitar el generador random
void obtenerDatos() {
  rpmNum = random(0, 201);
  rpmESP = String(rpmNum);

  tempNum = random(0, 101);
  tempESP = String(tempNum);

  voltNum = random(0, 37);
  voltESP = String(voltNum);

  ampNum = random(0, 8);
  ampESP = String(ampNum);

  throtNum = random(0, 101);
  throtESP = String(throtNum);

  latESP = "37.77519";
  lonESP = "-122.41994";
}