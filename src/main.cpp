/**********************************************************************************
*                                                                                 *
*  Design Synthesis.net -r.young 6/3/2019 v1.0                                    *
*  Job# 1280                                                                      *
*  JOBNAME = Laidley Skylight                                                     *
*  Master-Controller [DS-MSTR-CTLR]                                               *
*  Master Logic Controller                                                        *
*                                                                                 *
***********************************************************************************
*  UnitName = DS-MSTR-CTRL
*  Controls 2 SLAVE TWIN-MOTOR CONTROLLERs
***********************************************************************************/
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Automaton.h>
// *******************************************************************************
// Pins 
// *******************************************************************************
 #define masterSwitchPin     2
 #define statusLedPin        3
// Automaton Objects -------------------------------------------------------------
Atm_led statusLED;
Atm_button masterSwitch;
// State SYSTEM Flags -------------------------------------------------------------------
 bool SYSTEM_STOPPED{false};
 bool SYSTEM_CLOSED{false};
 bool SYSTEM_OPENED{false};
 bool SYSTEM_IN_MOTION{false};
 bool SYSTEM_ERROR_STATE{false};
 bool SYSTEM_ONLINE_OK{false};
// LF1 ---------------------------------------------------------------------------
 bool LF1_STOPPED{false};
 bool LF1_OPEN{false};
 bool LF1_CLOSED{false};
 bool LF1_MOVING{false};
 bool LF1_ERROR{false};
 bool LF1_ONLINE_OK{false};
// LF2 ---------------------------------------------------------------------------
 bool LF2_STOPPED{false};
 bool LF2_OPEN{false};
 bool LF2_CLOSED{false};
 bool LF2_MOVING{false};
 bool LF2_ERROR{false};
 bool LF2_ONLINE_OK{false};
// LF3 ---------------------------------------------------------------------------
 bool LF3_STOPPED{false};
 bool LF3_OPEN{false};
 bool LF3_CLOSED{false};
 bool LF3_MOVING{false};
 bool LF3_ERROR{false};
 bool LF3_ONLINE_OK{false};
// LF4 ----------------------------------------------------------------------------
 bool LF4_STOPPED{false};
 bool LF4_OPEN{false};
 bool LF4_CLOSED{false};
 bool LF4_MOVING{false};
 bool LF4_ERROR{false};
 bool LF4_ONLINE_OK{false};
// -------------------------------------------------------------------------------
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 10, 175);
IPAddress myDns(192, 168, 10, 199);
IPAddress gateway(192, 168, 10, 199);
IPAddress subnet(255, 255, 255, 0);
IPAddress mqttServer (192, 168, 10, 22);

EthernetClient ethclient;
PubSubClient client(ethclient);

unsigned long previousMillis;
unsigned long polling_interval = 1000;
int position = 0;

void CheckState(){

  if (LF1_ONLINE_OK && LF2_ONLINE_OK && LF3_ONLINE_OK && LF4_ONLINE_OK){
    if(!SYSTEM_ONLINE_OK){
    SYSTEM_ONLINE_OK=true;
    SYSTEM_STOPPED=false;
    SYSTEM_CLOSED=false;
    SYSTEM_OPENED=false;
    SYSTEM_IN_MOTION=false;
    Serial.println("SYSTEM ONLINE OK");
    }
  }
  if (LF1_STOPPED && LF2_STOPPED && LF3_STOPPED && LF4_STOPPED)
  {
    if(!SYSTEM_STOPPED){
    SYSTEM_STOPPED=true;
    SYSTEM_CLOSED=false;
    SYSTEM_OPENED=false;
    SYSTEM_IN_MOTION=false;
    Serial.println("SYSTEM STOPPED");
    }

  }
  if (LF1_CLOSED && LF2_CLOSED && LF3_CLOSED && LF4_CLOSED)
  {
    if (!SYSTEM_CLOSED) {
      SYSTEM_CLOSED=true;
      SYSTEM_OPENED=false;
      SYSTEM_IN_MOTION=false;
      SYSTEM_STOPPED=false;
      Serial.println("SYSTEM CLOSED");
    } 
  }

  if (LF1_OPEN && LF2_OPEN && LF3_OPEN && LF4_OPEN)
  {
    if(!SYSTEM_OPENED){
    SYSTEM_OPENED=true;
    SYSTEM_CLOSED=false;
    SYSTEM_IN_MOTION=false;
    SYSTEM_STOPPED=false;
    Serial.println("SYSTEM OPENED");
    }
  }
  if (LF1_MOVING && LF2_MOVING & LF3_MOVING && LF4_MOVING)
  {
    if(!SYSTEM_IN_MOTION){
    SYSTEM_IN_MOTION=true;
    SYSTEM_CLOSED=false;
    SYSTEM_OPENED=false;
    Serial.println("SYSTEM IN MOTION");
    }
  }

   if (LF1_ERROR || LF2_ERROR || LF3_ERROR || LF4_ERROR){
    if(!SYSTEM_ERROR_STATE){
    SYSTEM_IN_MOTION=false;
    SYSTEM_CLOSED=false;
    SYSTEM_OPENED=false;
    SYSTEM_STOPPED=false;
    Serial.println("SYSTEM ERROR");
    }
  }
  
}

void callback(char* topic, byte* payload, unsigned int length){

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  
  Serial.println();
  /*************************************************
   * LF1 STATUS
  **************************************************/
   if ((char)payload[0] == '1') {
   
    if ((char)payload[1] == '0'){
     LF1_STOPPED=true;
    }
    if ((char)payload[1] == '1'){
     LF1_CLOSED=true;
    }
    if ((char)payload[1] == '2'){
     LF1_OPEN=true;
    }
    if((char)payload[1] == '3'){
     LF1_MOVING=true;
    }
    if ((char)payload[1] == '4'){
     LF1_ERROR=true;
    }
    if ((char)payload[1] == '5'){
     LF1_ONLINE_OK=true;
    }
      
  } 
  /*************************************************
   * LF2 STATUS
  **************************************************/
  if((char)payload[0] == '2') {
   
    if ((char)payload[1] == '0'){
     LF2_STOPPED=true;
    }
    if ((char)payload[1] == '1'){
     LF2_CLOSED=true;
    }
    if ((char)payload[1] == '2'){
     LF2_OPEN=true;
    }
    if ((char)payload[1] == '3'){
     LF2_MOVING=true;
    }
    if ((char)payload[1] == '4'){
     LF2_ERROR=true;
    }
    if ((char)payload[1] == '5'){
     LF2_ONLINE_OK=true;
    }
      
  } 
  /*************************************************
   * LF3 STATUS
  **************************************************/
  if ((char)payload[0] == '3') {
   
    if ((char)payload[1] == '0'){
     LF3_STOPPED=true;
    }
    if ((char)payload[1] == '1'){
     LF3_CLOSED=true;
    }
    if ((char)payload[1] == '2'){
     LF3_OPEN=true;
    }
    if ((char)payload[1] == '3'){
     LF3_MOVING=true;
    }
    if ((char)payload[1] == '4'){
     LF3_ERROR=true;
    } 
    if ((char)payload[1] == '5'){
     LF3_ONLINE_OK=true;
    }  
  } 

  /*************************************************
   * LF4 STATUS
  **************************************************/
  if ((char)payload[0] == '4') {
   
    if ((char)payload[1] == '0'){
     LF4_STOPPED=true;
    }
    if ((char)payload[1] == '1'){
     LF4_CLOSED=true;
    }
    if ((char)payload[1] == '2'){
     LF4_OPEN=true;
    }
    if ((char)payload[1] == '3'){
     LF4_MOVING=true;
    }
    if ((char)payload[1] == '4'){
     LF4_ERROR=true;
    }
    if ((char)payload[1] == '5'){
     LF4_ONLINE_OK=true;
    }
       
  } 

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("LFMSTR")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("SYS", "1");
      
      // ... and resubscribe
      client.subscribe("STATUS");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void button_change( int idx, int v, int up ) {

  Serial.println("Button Pressed -> CLOSE");
  if (!SYSTEM_CLOSED){
    client.publish("SIGNAL","1");
  }
  
  
}

void button_release( int idx, int v, int up ) {

  Serial.println("Button Release -> OPEN");
  if(!SYSTEM_OPENED){
  client.publish("SIGNAL","2");
  }
  
}
 
void setup() {
  
  Serial.begin(9600);
  // print your local IP address: 
  // Allow the hardware to sort itself out
  delay(1500);

  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
  Ethernet.begin(mac, ip);
  Serial.println(Ethernet.localIP());
  masterSwitch.begin(masterSwitchPin);
  masterSwitch.onPress(button_change);
  masterSwitch.onRelease(button_release);
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
 
  client.loop();
  automaton.run();

  // Task Loop Interval 
  unsigned long currentMillis = millis();
  // Main Utility Task Loop
  if(currentMillis - previousMillis > polling_interval) {  
    previousMillis = currentMillis;  
    
    CheckState();
  }

}