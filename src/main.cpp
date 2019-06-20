/**********************************************************************************
*                                                                                 *
*  Design Synthesis.net -r.young 6/3/2019 v1.1                                   *
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
#include <EEPROM.h>

// *******************************************************************************
// Pins 
// *******************************************************************************
 #define masterSwitchPin    2
 #define downStatusLedPin   3
 #define connectionLedPin   4
 #define upStatusLedPin     5
// Automaton Objects -------------------------------------------------------------
Atm_led connectionLED;
Atm_led downStatusLed;
Atm_led upStatusLed;
Atm_button masterSwitch;

enum state{IDLE, STOPPED,RAISE,LOWERED,ERROR};

// State SYSTEM Flags -----------------------------------------------------------
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
int current_command{0};
enum state CURRENT_STATE;
enum state LAST_STATE;

void CheckState(){

  current_command=EEPROM.read(0);

  if (LF1_ONLINE_OK && LF2_ONLINE_OK && LF3_ONLINE_OK && LF4_ONLINE_OK){
    if(!SYSTEM_ONLINE_OK){

      CURRENT_STATE = IDLE;
    
    }
  }
  if (LF1_STOPPED && LF2_STOPPED && LF3_STOPPED && LF4_STOPPED)
  {
    if(!SYSTEM_STOPPED){
    
      CURRENT_STATE = STOPPED;
    }

  }
  if (LF1_CLOSED && LF2_CLOSED && LF3_CLOSED && LF4_CLOSED)
  {
    if (!SYSTEM_CLOSED) {
      
      CURRENT_STATE = LOWERED;
     
    } 
  }

  if (LF1_OPEN && LF2_OPEN && LF3_OPEN && LF4_OPEN)
  {
    if(!SYSTEM_OPENED){

     CURRENT_STATE = RAISE;
   
    }
  }
  if (LF1_MOVING && LF2_MOVING & LF3_MOVING && LF4_MOVING)
  {
    // if(!SYSTEM_IN_MOTION){
    // SYSTEM_IN_MOTION=true;
    // SYSTEM_CLOSED=false;
    // SYSTEM_OPENED=false;
    // SYSTEM_STOPPED=false;
    // Serial.println("SYSTEM IN MOTION");
    // if(command==1){
    //   downStatusLed.trigger(downStatusLed.EVT_BLINK  );
    // }
    // else if(command==2){
    //   upStatusLed.trigger(upStatusLed.EVT_BLINK  );
    // }
    // }
    
  }

   if (LF1_ERROR || LF2_ERROR || LF3_ERROR || LF4_ERROR){
    if(!SYSTEM_ERROR_STATE){
      
   

    }
   }
  
}

void StateMachine(){

  
  LAST_STATE = CURRENT_STATE;
  EEPROM.write(1,CURRENT_STATE);

  switch(CURRENT_STATE)
  {
      case IDLE:

        SYSTEM_ONLINE_OK=true;
        SYSTEM_STOPPED=false;
        SYSTEM_CLOSED=false;
        SYSTEM_OPENED=false;
        SYSTEM_IN_MOTION=false;
        Serial.println("SYSTEM ONLINE OK");        
        connectionLED.blink(40,2000).trigger(connectionLED.EVT_BLINK);
        break;
      
      case STOPPED:

        SYSTEM_STOPPED=true;
        SYSTEM_CLOSED=false;
        SYSTEM_OPENED=false;
        SYSTEM_IN_MOTION=false;
        Serial.println("SYSTEM STOPPED");
        downStatusLed.blink(800,800).trigger(downStatusLed.EVT_BLINK);
        upStatusLed.blink(800,800).trigger(upStatusLed.EVT_BLINK);
        
        break;
      
      case RAISE:

        SYSTEM_OPENED=true;
        SYSTEM_CLOSED=false;
        SYSTEM_IN_MOTION=false;
        SYSTEM_STOPPED=false;
        Serial.println("SYSTEM OPENED");
        upStatusLed.trigger(upStatusLed.EVT_ON);
        downStatusLed.trigger(downStatusLed.EVT_OFF);
        break;
      
      case LOWERED:
        
          SYSTEM_CLOSED=true;
          SYSTEM_OPENED=false;
          SYSTEM_IN_MOTION=false;
          SYSTEM_STOPPED=false;
          Serial.print("SYSTEM CLOSED");
          downStatusLed.trigger(downStatusLed.EVT_ON);
          upStatusLed.trigger(upStatusLed.EVT_OFF);
          break;
      
      case ERROR:

        SYSTEM_IN_MOTION=false;
        SYSTEM_CLOSED=false;
        SYSTEM_OPENED=false;
        SYSTEM_STOPPED=false;
        Serial.println("SYSTEM ERROR");
       break;
  
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
      connectionLED.begin(connectionLedPin).blink(40,2000).trigger(connectionLED.EVT_BLINK);
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

  
  if (!SYSTEM_CLOSED || current_command==1){
    Serial.println("Button Pressed -> CLOSE");
    client.publish("SIGNAL","1");
    current_command=1;
    EEPROM.write(0,current_command);
   
  }
  
  
}

void button_release( int idx, int v, int up ) {

 
  if(!SYSTEM_OPENED || current_command==2){
  Serial.println("Button Release -> OPEN");
  client.publish("SIGNAL","2");
  current_command=2;
  EEPROM.write(0,current_command);
 
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
  connectionLED.begin(connectionLedPin);
  downStatusLed.begin(downStatusLedPin).blink(20,400);
  upStatusLed.begin(upStatusLedPin).blink(20,400);

  CURRENT_STATE = (state)EEPROM.read(1);
  
  current_command = EEPROM.read(0);
  
  
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
    if(CURRENT_STATE != LAST_STATE){
    StateMachine();
    }
  }

}