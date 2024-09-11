//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 05 ESP32 Master OLED Buttons Lora Ra-02 Final

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  --------------------------------------                                                                                                            //
//  Description of how this project works.                                                                                                            //
//                                                                                                                                                    //
//  - The master sends a broadcast message to the slaves with a certain address every 1 second.                                                       //
//                                                                                                                                                    //
//  - The contents of the broadcast message from the Master are: Slave_Address/Destination | Address_Master/Sender | Message/Data                     //
//    (Message/Data contains commands to control the LEDs on the slaves.)                                                                             //
//                                                                                                                                                    //
//  - Example :                                                                                                                                       //
//    Broadcast message content : 0x02 | 0x01 | f,f                                                                                                   //
//    > 0x02 = Slave_Address/Destination.                                                                                                             //
//    > 0x01 = Master/Sender_Address.                                                                                                                 //
//    > f,f = "f" the first one to control the LED on Slave 1,"f' the second one to control the LED on Slave 2.                                       //
//    > "t" to turn on the LED, "f" to turn off the LED.                                                                                              //
//                                                                                                                                                    //
//  - The slaves received a broadcast message from the master.                                                                                        //
//                                                                                                                                                    //
//  - The Slaves will check whether the broadcast message sent by the Master is for Slave 1 or Slave 2.                                               //
//                                                                                                                                                    //
//  - If the broadcast message sent by the Master is not for the intended slave, then the slave will ignore the broadcast message,                    //
//    but the content of the broadcast message is still retrieved for processing,                                                                     //
//    because the content of the broadcast message is to control the LEDs on the slaves.                                                              //
//                                                                                                                                                    //
//  - If the broadcast message sent by the Master is indeed for the intended slave,                                                                   //
//    then the slave will take the contents of the broadcast message to be processed,                                                                 //
//    because the content of the broadcast message is to control the LEDs on the slaves,                                                              //
//    then the slave will send a message back to the Master containing humidity value, temperature value and the last state of the LED.               //
//  --------------------------------------                                                                                                            //
//                                                                                                                                                    //
//  --------------------------------------                                                                                                            //
//  An overview of the workings of communication between the Master and the Slaves.                                                                   //
//                                                                                                                                                    //
//  - Master sends a broadcast message with the destination address is Slave 1.                                                                       //
//  - 1 second delay.                                                                                                                                 //
//  - Slave 1 receives a broadcast message,                                                                                                           //
//    Slave 1 processes the contents of the broadcast message to take orders to control the LED, because the message is intended for Slave 1,         //
//    Slave 1 sends a message back to the Master containing the data on humidity, temperature and the last state of the LED.                          //
//  - Slave 2 receives the broadcast message,                                                                                                         //
//    Slave 2 processes the contents of the broadcast message to take the command to control the LED,                                                 //
//    because the message is not intended for Slave 2, it does not send a message back to the Master.                                                 //
//  - Master received a message back from Slave 1.                                                                                                    //
//                                                                                                                                                    //
//  - Master sends a broadcast message with the destination address is Slave 2.                                                                       //
//  - 1 second delay.                                                                                                                                 //
//  - Slave 1 receives the broadcast message,                                                                                                         //
//    Slave 1 processes the contents of the broadcast message to take the command to control the LED,                                                 //
//    because the message is not intended for Slave 1, it does not send a message back to the Master.                                                 //
//  - Slave 2 receives a broadcast message,                                                                                                           //
//    Slave 2 processes the contents of the broadcast message to take orders to control the LED, because the message is intended for Slave 2,         //
//    then Slave 2 sends a message back to the Master containing data on humidity, temperature and the last state of the LED.                         //
//  - Master received a message back from Slave 2.                                                                                                    //
//  --------------------------------------                                                                                                            //
//                                                                                                                                                    //
//  --------------------------------------                                                                                                            //
//  Additional information :                                                                                                                          //
//                                                                                                                                                    //
//  If you are sure that you have connected the Lora Ra-02 correctly to the ESP32,                                                                    //
//  but you get the error message "LoRa init failed ! Check your connections", then try using an external power source for the Lora Ra-02.            //
//  In this project I used a 3.3V power supply from an ESP32 board to power the Lora Ra-02.                                                           //
//  Because maybe the quality of the ESP32 board module is different in the market and the power supply is not strong enough to power the Lora Ra-02. //
//  So you get the error message "LoRa init failed ! Check your connections".                                                                         //
//  --------------------------------------                                                                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------- Include Library.
#include <SPI.h>
#include <LoRa.h>
//---------------------------------------- 
//---------------------------------------- 

//---------------------------------------- Defines the Pins for the LEDs.
#define LED_Send_Pin    4 //--> D4
#define LED_Receive_Pin 15 //--> D15
//---------------------------------------- 
//----------------------------------------
//----------------------------------------

//---------------------------------------- LoRa Pin / GPIO configuration.
#define ss 5
#define rst 14
#define dio0 2
//----------------------------------------

//---------------------------------------- Variable declaration to hold incoming and outgoing data.
String Incoming = "";
String Message = "";             
//----------------------------------------

//---------------------------------------- LoRa data transmission configuration.
byte LocalAddress = 0x01;               //--> address of this device (Master Address).
byte Destination_ESP32_Slave_1 = 0x02;  //--> destination to send to Slave 1 (ESP32).
byte Destination_ESP32_Slave_2 = 0x03;  //--> destination to send to Slave 2 (ESP32).
//---------------------------------------- 

//---------------------------------------- Variable declaration for Millis/Timer.
unsigned long previousMillis_SendMSG = 0;
const long interval_SendMSG = 1000;

unsigned long previousMillis_RestartLORA = 0;
const long interval_RestartLORA = 1000;
//---------------------------------------- 

//---------------------------------------- Variable declaration to control the LEDs on the Slaves.
bool LED_1_State_Slave_1 = false;
bool LED_1_State_Slave_2 = false;
//---------------------------------------- 

//---------------------------------------- Variable declaration to display the state of the LEDs on the Slaves in the OLED LCD.
/*
bool LED_1_State_Disp_Slave_1 = false;
bool LED_1_State_Disp_Slave_2 = false;
*/
//---------------------------------------- 

//---------------------------------------- Variable declaration to get temperature and humidity values received from Slaves.

struct dataPack
{
  float voltage; 
  int temp;
  float speed;
  float amps; 
  float latitude[3];
  float longitude[3]; 
  float rpm; 
  bool deathMan; 
  float throttle; 
}; 

dataPack receiveData; 
//---------------------------------------- 

// Variable declaration to count slaves.
byte Slv = 0;

// Variable declaration to get the address of the slaves.
byte SL_Address;

// Variable declaration to get LED state on slaves.
String LEDs_State = "";

// Declaration of variable as counter to restart Lora Ra-02.
byte Count_to_Rst_LORA = 0;

//________________________________________________________________________________ Subroutines for sending data (LoRa Ra-02).
void sendMessage(String Outgoing, byte Destination) {
  //digitalWrite(LED_Send_Pin, HIGH);    // Turn on Send LED when sending data
  LoRa.beginPacket();             //--> start packet
  LoRa.write(Destination);        //--> add destination address
  LoRa.write(LocalAddress);       //--> add sender address
  LoRa.write(Outgoing.length());  //--> add payload length
  LoRa.print(Outgoing);           //--> add payload
  LoRa.endPacket();               //--> finish packet and send it
  delay(100);                     // Small delay to ensure data is sent
  //digitalWrite(LED_Send_Pin, LOW);     // Turn off Send LED after sending data
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subroutines for receiving data (LoRa Ra-02).
void onReceive(int packetSize) {
  if (packetSize == 0) 
  {
    return;
  }// if there's no packet, return

  //digitalWrite(LED_Receive_Pin, HIGH);  // Turn on Receive LED when receiving data

  //---------------------------------------- read packet header bytes:
  int recipient = LoRa.read();        //--> recipient address
  byte sender = LoRa.read();          //--> sender address
  byte incomingLength = LoRa.read();  //--> incoming msg length
  //---------------------------------------- 

  // Get the address of the senders or slaves.
  SL_Address = sender;

  // Clears Incoming variable data.
  Incoming = "";

  //---------------------------------------- Get all incoming data / message.
  while (LoRa.available()) {
    Incoming += (char)LoRa.read();
  }
  //---------------------------------------- 

  // Resets the value of the Count_to_Rst_LORA variable if a message is received.
  Count_to_Rst_LORA = 0;

  //---------------------------------------- Check length for error.
  if (incomingLength != Incoming.length()) {
    Serial.println();
    Serial.println("er"); //--> "er" = error: message length does not match length.
    Serial.println("error: message length does not match length");
    //digitalWrite(LED_Receive_Pin, LOW);  // Turn off Receive LED
    return; //--> skip rest of function
  }
  //---------------------------------------- 

  //---------------------------------------- Checks whether the incoming data or message for this device.
  if (recipient != LocalAddress) {
    Serial.println();
    Serial.println("!");  //--> "!" = This message is not for me.
    Serial.println("This message is not for me.");
    digitalWrite(LED_Receive_Pin, LOW);  // Turn off Receive LED
    return; //--> skip rest of function
  }
  //---------------------------------------- 

  //----------------------------------------  if message is for this device, or broadcast, print details:
  Serial.println();
  Serial.println("Rc from: 0x" + String(sender, HEX));
  Serial.println("Message: " + Incoming);
  //---------------------------------------- 

  // Calls the Processing_incoming_data() subroutine.
  Processing_incoming_data();

  //digitalWrite(LED_Receive_Pin, LOW);  // Turn off Receive LED after processing data
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subroutines to process data from incoming messages.
//  - The message sent by the slaves contained data like this: Humidity_Value,Temperature_Value,Last_State of_LED
//
//  - Example : 80,30.50,0
//    > 80 is the humidity value.
//    > 30.50 is the temperature value.
//    > 0 is the last state of the LED (0 means the LED is off and 1 means the LED is on).
//
//  - To separate data as in the example above, the "GetValue" subroutine is used, using the separator ",".
// 
//  - Example :
//    Incoming = "80,30.50,0".
//    > 80 is the first data or "0" if using the "GetValue" subroutine.
//    > 30.50 is the second data or "1" if using the "GetValue" subroutine.
//    > 0 is the third data or "2" if using the "GetValue" subroutine.
//
//  - So to retrieve the data above is like this:
//    > Humdity = GetValue(Incoming, ',', 0).toInt();
//    > Temperature = GetValue(Incoming, ',', 1).toFloat();
//    > LEDs_State = GetValue(Incoming, ',', 2);

void Processing_incoming_data() {
  //---------------------------------------- Conditions for processing data or messages from Slave 1 (ESP32 Slave 1).
  if (SL_Address == Destination_ESP32_Slave_1) {
    receiveData.voltage = GetValue(Incoming, ',', 0).toFloat(); 
    receiveData.amps = GetValue(Incoming, ',', 1).toFloat(); 
    receiveData.temp = GetValue(Incoming, ',', 2).toInt(); 
    receiveData.latitude[0] = GetValue(Incoming, ',', 3).toFloat(); 
    receiveData.latitude[1] = GetValue(Incoming, ',', 4).toFloat(); 
    receiveData.latitude[2] = GetValue(Incoming, ',', 5).toFloat(); 
    receiveData.longitude[0] = GetValue(Incoming, ',', 6).toFloat(); 
    receiveData.longitude[1] = GetValue(Incoming, ',', 7).toFloat(); 
    receiveData.longitude[2] = GetValue(Incoming, ',', 8).toFloat(); 
    receiveData.longitude[3] = GetValue(Incoming, ',', 9).toFloat(); 
    receiveData.rpm = GetValue(Incoming, ',', 10).toInt(); 
    receiveData.deathMan = GetValue(Incoming, ',', 11).toInt(); 
    receiveData.throttle = GetValue(Incoming, ',', 12).toFloat(); 
  }
  //---------------------------------------- 
}
//_______________________________________________________________________________

//________________________________________________________________________________ Subroutine to get messages from slaves containing the latest data.
//  - The master sends a message to the slaves.
//  - The slaves reply to the message from the master by sending a message containing the latest data of temperature, humidity and LED state to the Master.

void Getting_data_for_the_first_time() {
  Serial.println();
  Serial.println("Getting data for the first time...");

  //---------------------------------------- Loop to get data for the first time.
  while(true) {
    unsigned long currentMillis_SendMSG = millis();

    if (currentMillis_SendMSG - previousMillis_SendMSG >= interval_SendMSG) {
      previousMillis_SendMSG = currentMillis_SendMSG;

      Slv++;
      if (Slv > 2) {
        Slv = 0;
        Serial.println();
        Serial.println("Getting data for the first time has been completed.");
        break;
      }
  
      Message = "N,N";

      //::::::::::::::::: Condition for sending message / command data to Slave 1 (ESP32 Slave 1).
      if (Slv == 1) {
        Serial.println();
        Serial.print("Send message to ESP32 Slave " + String(Slv));
        Serial.println(" for first time : " + Message);
        sendMessage(Message, Destination_ESP32_Slave_1);
      }
      //::::::::::::::::: 

      //::::::::::::::::: Condition for sending message / command data to Slave 2 (ESP32 Slave 2).
      if (Slv == 2) {
        Serial.println();
        Serial.print("Send message to ESP32 Slave " + String(Slv));
        Serial.println(" for first time : " + Message);
        sendMessage(Message, Destination_ESP32_Slave_2);
      }
      //::::::::::::::::: 
    }

    //---------------------------------------- parse for a packet, and call onReceive with the result:
    onReceive(LoRa.parsePacket());
    //----------------------------------------
  }
  //---------------------------------------- 
}
//________________________________________________________________________________ 

//________________________________________________________________________________ String function to process the data received
// I got this from : https://www.electroniclinic.com/reyax-lora-based-multiple-sensors-monitoring-using-arduino/
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
//________________________________________________________________________________ 

//________________________________________________________________________________ Subroutine to reset Lora Ra-02.
void Rst_LORA() {
  LoRa.setPins(ss, rst, dio0);

  Serial.println();
  Serial.println("Restart LoRa...");
  Serial.println("Start LoRa init...");
  if (!LoRa.begin(433E6)) {             // initialize ratio at 915 or 433 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  Serial.println("LoRa init succeeded.");

  // Reset the value of the Count_to_Rst_LORA variable.
  Count_to_Rst_LORA = 0;
}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID SETUP
void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  pinMode(Button_1_Pin, INPUT);
  pinMode(Button_2_Pin, INPUT);
  //pinMode(LED_Send_Pin, OUTPUT);    // Initialize Send LED pin as output
  //pinMode(LED_Receive_Pin, OUTPUT); // Initialize Receive LED pin as output

  //---------------------------------------- Clears the values of the Temp and Humd array variables for the first time.
  receiveData.temp = 0.0; 
  receiveData.amps = 0.0; 
  receiveData.voltage = 0.0;
  receiveData.rpm = 0; 
  receiveData.speed = 0.0; 
  receiveData.deathMan = false; 
  receiveData.throttle = 0.0; 
  for (byte i = 0; i < 3; i++) {
    latitude[i] = 0.0;
    longitude[i] = 0.0;
  }
  //---------------------------------------- 

  delay(1000);

  // Calls the Rst_LORA() subroutine.
  Rst_LORA();

  // Calls the Getting_data_for_the_first_time() subroutine.
  Getting_data_for_the_first_time();
}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID LOOP
void loop() {
  // put your main code here, to run repeatedly:
  //---------------------------------------- Millis/Timer to send messages to slaves every 1 second (see interval_SendMSG variable).
  //  Messages are sent every one second is alternately.
  //  > Master sends message to Slave 1, delay 1 second.
  //  > Master sends message to Slave 2, delay 1 second.
  
  /*
  unsigned long currentMillis_SendMSG = millis();
  
  if (currentMillis_SendMSG - previousMillis_SendMSG >= interval_SendMSG) {
    previousMillis_SendMSG = currentMillis_SendMSG;

    //::::::::::::::::: The condition to update the OLED LCD if there is no incoming message from all slaves.
    Count_OLED_refresh_when_no_data_comes_in++;
    if (Count_OLED_refresh_when_no_data_comes_in > 5) {
      Count_OLED_refresh_when_no_data_comes_in = 0;
      Processing_incoming_data();
    }
    //::::::::::::::::: 

    //::::::::::::::::: Count the slaves.
    Slv++;
    if (Slv > 2) Slv = 1;
    //::::::::::::::::: 
    /*
    //::::::::::::::::: The condition for setting the variable used to change the state of the LEDs on the slaves.
    // "t" to turn on the LED.
    // "f" to turn off the LED.

    String send_LED_1_State_Slave_1 = "";
    String send_LED_1_State_Slave_2 = "";
    
    if (LED_1_State_Slave_1 == true) send_LED_1_State_Slave_1 = "t";
    if (LED_1_State_Slave_1 == false) send_LED_1_State_Slave_1 = "f";
    if (LED_1_State_Slave_2 == true) send_LED_1_State_Slave_2 = "t";
    if (LED_1_State_Slave_2 == false) send_LED_1_State_Slave_2 = "f";
    //::::::::::::::::: 

    // Enter the values of the send_LED_1_State_Slave_1 and send_LED_1_State_Slave_2 variables to the Message variable.
    Message = send_LED_1_State_Slave_1 + "," + send_LED_1_State_Slave_2;

    //::::::::::::::::: Condition for sending message / command data to Slave 1 (ESP32 Slave 1).
    if (Slv == 1) {
      Serial.println();
      Serial.println("Tr to  : 0x" + String(Destination_ESP32_Slave_1, HEX));
      Serial.println("Message: " + Message);
      Humd[0] = 0;
      Temp[0] = 0.00;
      sendMessage(Message, Destination_ESP32_Slave_1);
    }
    //::::::::::::::::: 
    
    //::::::::::::::::: Condition for sending message / command data to Slave 2 (ESP32 Slave 2).
    if (Slv == 2) {
      Serial.println();
      Serial.println("Tr to  : 0x" + String(Destination_ESP32_Slave_2, HEX));
      Serial.println("Message: " + Message);
      Humd[1] = 0;
      Temp[1] = 0.00;
      sendMessage(Message, Destination_ESP32_Slave_2);
    }
    
    //::::::::::::::::: 
  }
  */
  //---------------------------------------- 

  //---------------------------------------- Millis/Timer to reset Lora Ra-02.
  //  - Lora Ra-02 reset is required for long term use.
  //  - That means the Lora Ra-02 is on and working for a long time.
  //  - From my experience when using Lora Ra-02 for a long time, there are times when Lora Ra-02 seems to "freeze" or an error, 
  //    so it can't send and receive messages. It doesn't happen often, but it does happen sometimes. 
  //    So I added a method to reset Lora Ra-02 to solve that problem. As a result, the method was successful in solving the problem.
  //  - This method of resetting the Lora Ra-02 works by checking whether there are incoming messages, 
  //    if no messages are received for approximately 30 seconds, then the Lora Ra-02 is considered to be experiencing "freezing" or error, so a reset is carried out.

  unsigned long currentMillis_RestartLORA = millis();
  if (currentMillis_RestartLORA - previousMillis_RestartLORA >= interval_RestartLORA) {
    previousMillis_RestartLORA = currentMillis_RestartLORA;

    Count_to_Rst_LORA++;
    if (Count_to_Rst_LORA > 30) {
      LoRa.end();
      Rst_LORA();
    }
  }
  //----------------------------------------
  //delay(1000); //A delay may be implemented if needed 

  //----------------------------------------parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
  //----------------------------------------
}
//________________________________________________________________________________ 
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/*
#################################################################################################
#################################################################################################
#################################################################################################
################################################################################################# */
