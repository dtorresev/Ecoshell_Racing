#include <Wire.h>
#include <TinyGPS++.h>

#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);
char   datoCmd = 0;

#define NMEA 0
TinyGPSPlus gps;
// chuy se la come
// @dtorresev se la traga
//------------------------------------------------------------------------------------------------------------------------------------------

void setup() 
{
  Serial.begin(115200);
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2); 
  delay(2000);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void loop() 
{

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
