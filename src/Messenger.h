
#ifndef Messenger_h
#define Messenger_h
#define MESSENGERBUFFERSIZE 64

#include <inttypes.h>
#include "Arduino.h"
#include "commands.h"

extern "C" {
// callback function
    typedef void (*messengerCallbackFunction)(void);
}


class Messenger
{

public:
 Messenger();
  Messenger(char separator);
  int readInt();
  long readLong(); // Added based on a suggestion by G. Paolo Sani
  char readChar();
  double readDouble(); // Added based on a suggestion by Lorenzo C.
  void copyString(char *string, uint8_t size);
  uint8_t checkString(char *string);
  uint8_t process(int serialByte);
  uint8_t available();
  void attach(messengerCallbackFunction newFunction);
  void attachCommands(cmdlist_t* commands);
  int handleData(char cmdline[]);
  
private:
  void init(char separator);
  uint8_t next();
  void reset();

  void handleParams(char *param, const char sep);
  messengerCallbackFunction messageCompleted();

  uint8_t messageState;
  
  messengerCallbackFunction callback;
  
  char* current; // Pointer to current data
  char* last;
  
  char token[2];
  uint8_t dumped;
  
  uint8_t bufferIndex; // Index where to write the data
  char buffer[MESSENGERBUFFERSIZE]; // Buffer that holds the data
  uint8_t bufferLength; // The length of the buffer (defaults to 64)
  uint8_t bufferLastIndex; // The last index of the buffer

  cmdlist_t* cmdlist;
  //char data[DATA_MAX_SIZE];   // an array to store the received data
  char* args[MAX_ARGS + 1];
};

#endif
