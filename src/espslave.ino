#define MAX_SWITCHES 10
#define RF_RX_PIN 0 // D2
#define RF_TX_PIN 3 // D10

#define BAUDRATE 9600

#include "remotecontroller.h"
//#include "TasmotaSlave.h"
#include "Messenger.h"

#ifdef HAS_DISPLAY
#include <LCDIC2.h>
#include "display.h"
#endif

#ifdef HAS_THERMISTOR
#define THERMISTOR_PIN A0 //A7
#endif

#define STATE_STANDBY 0
#define STATE_REC     1
#define STATE_REC_OFF 2
#define STATE_SKIP_NEXT 3
static int currentState = STATE_STANDBY;
int currentSwitch = 0;

// Instantiate Messenger object with the message function and the default separator
// (the space character)
static const Messenger message = Messenger(';');

//SoftwareSerial espSerial(SERIAL_RX_PIN, SERIAL_TX_PIN); // RX, TX
//TasmotaSlave slave(&Serial);

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 1000;           // interval at which to blink (milliseconds)

RCSwitch rfReceiver = RCSwitch();
RCSwitch rfEmitter = RCSwitch();

RemoteController controller = RemoteController(&rfReceiver, &rfEmitter);

#ifdef HAS_DISPLAY
LCDIC2 lcd(0x27, 16, 2);
Display disp(&lcd);
#endif

void set(char* param[]) {
  /*
    Serial.print("command: set, ");
    Serial.println("params: ");
    for (int x=0; param[x]; x++) {
    Serial.print(x);
    Serial.print(" - ");
    Serial.println(param[x]);
    }
  */
  switch_t *s = controller.getSwitch(atoi(param[0]));
  s->value_on = strtol(param[1], NULL, 16);  
  if (strlen(param[2])==0) {
    s->value_off = 0xFFFFFF;        
  } else {
    s->value_off = strtol(param[2], NULL, 16);      
  }
  controller.saveSwitch(atoi(param[0]));
  char response[100];
  sprintf(response,"{\"switch\": %s, \"action\": \"set\", \"on\": %lp, \"off\": %lp}", param[0], s->value_on, s->value_off);        
  Serial.print(response);
}

void setType(char* param[]) {
  switch_t *s = controller.getSwitch(atoi(param[0]));
  s->type = atoi(param[1]);
  controller.saveSwitch(atoi(param[0]));
}

void get(char* param[]) {
  switch_t *s = controller.getSwitch(atoi(param[0]));
  char response[100];
  sprintf(response,"{\"switch\": %s, \"action\": \"get\", \"on\": %lp, \"off\": %lp}", param[0], s->value_on, s->value_off);        
  Serial.print(response);
}

void power(char* param[]) {    
  unsigned long value = strtol(param[0], NULL, 16);
  if (value < MAX_SWITCHES) {
    char response[100];
    switch_t *s = controller.getSwitch(atoi(param[0]));
    value = (strlen(param[1])==0 || atoi(param[1]) != 0 || !strcmp("off", param[1]))? s->value_on: 
      ((s->value_off!= 0xFFFFFF)? s->value_off: s->value_on);
    controller.sendRF(value);
    delay(100);
    currentState = STATE_SKIP_NEXT;
    sprintf(response,"{\"switch\": %s, \"action\": \"send\", \"state\": \"%s\", \"rfcode\": %lp}", param[0], value == s->value_on? "on": "off", value);        
    Serial.print(response);
  }
}

void send(char* param[]) {    
  char response[100];
  unsigned long value = strtol(param[0], NULL, 16);
  controller.sendRF(value);
  delay(100);
  currentState = STATE_SKIP_NEXT;
  sprintf(response,"{\"action\": \"send\", \"rfcode\": %lp}", value);        
  Serial.print(response);  
}

void rec(char* param[]) {
  bool recorded = false;
  currentSwitch = atoi(param[0])-1;
  switch_t *s = controller.getSwitch(currentSwitch);
  
  if (strlen(param[1])==0 || atoi(param[1]) != 0 || !strcmp("off", param[1])) {
    char response[100];
    sprintf(response,"{\"switch\": %s, \"action\": \"rec\", \"state\": \"on\"}", param[0]); 
    Serial.print(response);    
    currentState = STATE_REC;
  } else {
    //Serial.println(param[1]);
    currentState = STATE_REC_OFF;
    char response[100];
    sprintf(response,"{\"switch\": %s, \"action\": \"rec\", \"state\": \"off\"}", param[0]); 
    Serial.print(response);
  }
}

void print(char* param[]) {
  switch_t *s;
  Serial.print("[");
  for (int i = 0; i < MAX_SWITCHES; i++) {
    s = controller.getSwitch(i);
    Serial.print("{\"switch\": ");
    Serial.print(i);
    Serial.print(", \"on\": 0x");
    Serial.print(s->value_on, HEX);
    Serial.print(", \"off\": 0x");
    Serial.print(s->value_off, HEX);
    Serial.print(" }");
    if (i < MAX_SWITCHES-1) {
    Serial.print(", ");      
    }
  }
  Serial.print("]");
}

//int Vo;
//float R1 = 10000;
//float logR2, R2, Tc;
//float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

double Thermister(int RawADC) {  //Function to perform the fancy math of the Steinhart-Hart equation
 double Temp;
 Temp = log(((10240000/RawADC) - 10000));
 Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
 Temp = Temp - 273.15;              // Convert Kelvin to Celsius
 //Temp = (Temp * 9.0)/ 5.0 + 32.0; // Celsius to Fahrenheit - comment out this line if you need Celsius
 return Temp;
}

void gpio(char* param[]) {
  char response[100];
  unsigned int pin = atoi(param[0]);
  if (pin > 3 && pin <= 13) {
    if (strlen(param[1])) {
      unsigned int value = atoi(param[1])? HIGH: LOW;
      digitalWrite(pin, value);
      sprintf(response,"{\"gpio\": %d, \"action\": \"write\", \"value\": %d }", pin, value); 
    } else {
      unsigned int value = digitalRead(pin);
      sprintf(response,"{\"gpio\": %d, \"action\": \"read\", \"value\": %d }", pin, value);     
    }
    Serial.print(response); 
  }
}  
#ifdef HAS_THERMISTOR
void temp(char* param[]) {
  char response[100];
  int Vo = analogRead(THERMISTOR_PIN);
  //R2 = R1 * (1023.0 / (float)Vo - 1.0);
  //logR2 = log(R2);
  //Tc = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  //Tc = Tc - 273.15;
  double Tc = Thermister(Vo);  
  //Tf = (Tc * 9.0)/ 5.0 + 32.0; 

  sprintf(response,"{\"temperature\": %d.%d }", int(Tc), int(((signed long)(Tc*100))%100)); 
  Serial.print(response);
}  
#endif

cmdlist_t cmdlist[] = {
  { "rec",    rec  },
  { "set",    set  },
  { "type",   setType  },
  { "get",    get  },
  { "power", power },
  { "send",   send },
  { "print",  print },
#ifdef HAS_THERMISTOR  
  { "temp",   temp },
#endif
  { "gpio",   gpio },
  { NULL,   NULL }
};

void setup() {
  Serial.begin(BAUDRATE);
  pinMode(2, INPUT);
  pinMode(RF_TX_PIN, OUTPUT);
  digitalWrite(RF_TX_PIN, LOW);

  #ifdef HAS_THERMISTOR
  pinMode(THERMISTOR_PIN, INPUT);
  #endif

  #ifdef HAS_DISPLAY
  if (lcd.begin()) {
    //lcd.setBacklight(false);
    disp.setLine("Hello, also this.", 0);
    disp.setLine("World! This is a very line.", 1);
  }
  #endif

  int i;

  for (i=4; i<11; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);    
  }
  
  rfEmitter.enableTransmit(RF_TX_PIN);
  // Optional set protocol (default is 1, will work for most outlets)
  // rfEmitter.setProtocol(2);

  // Optional set pulse length.
  // rfEmitter.setPulseLength(320);
  
  // Optional set number of transmission repetitions.
  rfEmitter.setRepeatTransmit(15);
  
  //rfEmitter.disableReceive();
  rfReceiver.enableReceive(RF_RX_PIN);  // Receiver on interrupt 0 => that is pin #2

  message.attachCommands((cmdlist_t*)&cmdlist);
  
  // Attach the function which will respond to the JSON request from Tasmota
  //slave.attach_FUNC_JSON(user_FUNC_JSON);
  
  // Attach the function which will be called when the Tasmota device sends data using SlaveSend command
  //slave.attach_FUNC_COMMAND_SEND(user_FUNC_RECEIVE);  
}

#ifdef TASMOTA_SLAVE
/*******************************************************************\
 * Function which will be called when this slave device receives a
 * SlaveSend command from the Tasmota device.
\*******************************************************************/

void user_FUNC_RECEIVE(char *data)
{
  //Serial.print("Received ");
  //Serial.println(data);
  message.handleData( data );
}

/*******************************************************************\
 * user_FUNC_JSON creates the JSON which will be sent back to the
 * Tasmota device upon receiving a request to do so
\*******************************************************************/
/*
void user_FUNC_JSON(void)
{
  uint8_t a = 0;
  char myjson[100];
  sprintf(myjson,"{\"A0\": %u, \"A1\": %u, \"A2\": %u, \"A3\": %u, \"A4\": %u, \"A5\": %u, \"A6\": %u, \"A7\": %u}", analogRead(A0), analogRead(A1), analogRead(A2), analogRead(A3), analogRead(A4), analogRead(A5), analogRead(A6), analogRead(A7));
  slave.sendJSON(myjson);
}
*/
#endif

void loop() {
  //slave.loop(); // Call the slave loop function every so often to process incoming requests
  if (Serial.available()) message.process(Serial.read());

  unsigned long value = NULL;
  char response[100];
  
  if (controller.RFAvailable()) {
    value = controller.getReceivedRFValue();
    controller.resetRFAvailable();
  }
  if (value != 0) {
    switch (currentState) {
      case STATE_REC:
        //Serial.print("Saved ");
        //Serial.println(value, HEX);
        controller.getSwitch(currentSwitch)->value_on = value;
        controller.saveSwitch(currentSwitch);
        currentState = STATE_STANDBY;
        sprintf(response,"{\"switch\": %d, \"state\": \"on\", \"rfcode\": %lp}", currentSwitch, value);
        //slave.SendTele(response);
        Serial.print(response);
        
        break;
      case STATE_REC_OFF:
        //Serial.print("Saved ");
        //Serial.println(value, HEX);
        controller.getSwitch(currentSwitch)->value_off = value;
        controller.saveSwitch(currentSwitch);
        currentState = STATE_STANDBY;
        sprintf(response,"{\"switch\": %d, \"state\": \"off\", \"rfcode\": %lp}", currentSwitch, value);        
        //slave.SendTele(response);
        Serial.print(response);
        
        break;
      case STATE_SKIP_NEXT:
        currentState = STATE_STANDBY;
        break;
      case STATE_STANDBY:
      default:
        //Serial.print("Received ");
        //Serial.println(value);
        sprintf(response,"{\"rfcode\": %lp}", value );  
        Serial.println(value, HEX);
        //slave.SendTele(response);
        Serial.print(response);
        
        break;
    }
  }
  #ifdef HAS_DISPLAY
  disp.update();  
  #endif
}
