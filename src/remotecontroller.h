#ifndef RemoteController_h
#define RemoteController_h

#include <inttypes.h>
#include <RCSwitch.h>

#ifndef MAX_SWITCHES
#define MAX_SWITCHES 10
#endif

#ifndef RF_RX_PIN
#define RF_RX_PIN 0 // D2
#endif

#ifndef RF_TX_PIN
#define RF_TX_PIN 3 // D10
#endif

#define SWITCH_TYPE_TOGGLE 0
#define SWITCH_TYPE_ONOFF 1

typedef struct datablock_t {
  unsigned short bHigh;
  unsigned short bMid;
  unsigned short bLow;
  unsigned short state;
} datablock_t;

typedef struct switch_t {
  int type;
  bool active;
  unsigned long value_on;
  unsigned long value_off;
  int state;
} switch_t;

class RemoteController {
public:
    RemoteController(RCSwitch* rx, RCSwitch* tx);
    switch_t *getSwitch(int count);
    void saveSwitch(int count);

    bool RFAvailable();
    void resetRFAvailable();
    void sendRF(unsigned long code);
    void switchById(int device_id, int onoff);
    unsigned long getReceivedRFValue();

private:
    RCSwitch* receiver;
    RCSwitch* transmitter;
    switch_t currentSwitch;
    datablock_t memBlock;
    unsigned long lastTimestamp;
    unsigned long lastValue;
    void processOwnOrExternalSignal(unsigned long code);

};

#endif
