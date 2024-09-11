#define TXD_PIN 17
#define RXD_PIN 16
float temp;
float vel;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    Serial2.begin(115200,SERIAL_8N1,RXD_PIN,TXD_PIN);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial2.available()){ 
    float tiempo = micros() / 1e6;
      //Se ponen los datos en variables
      temp = Serial2.parseFloat();
      vel = Serial2.parseFloat();

      //Se imprimen en el Serial para la graficación en Tiempo Real
      Serial.println(tiempo);
      Serial.println(temp);
      Serial.println(vel);

  }
}


