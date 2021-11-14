//ADDED FOR COMPATIBILITY WITH WIRING
extern "C" {
  #include <stdlib.h>
}

#include <EEPROM.h>
#include "remotecontroller.h"

RemoteController::RemoteController(RCSwitch* rx, RCSwitch* tx) {
    transmitter = tx;
    receiver = rx;
}

switch_t *RemoteController::getSwitch(int count) {
  int address = (count-1) * sizeof(datablock_t);
  EEPROM.get(address, memBlock);
  switch_t *s = &currentSwitch;
  s->value_on = ((unsigned long)memBlock.bHigh << 8) + (memBlock.bMid >> 8);
  s->value_off = ((unsigned long)(memBlock.bMid & 0xFF) << 16) + memBlock.bLow;
  s->type = memBlock.state >> 1;
  s->active = memBlock.state & 1;
  return s;
}

void RemoteController::saveSwitch(int count) {
  int address = (count-1) * sizeof(datablock_t);
  switch_t *s = &currentSwitch;
  memBlock.bHigh = s->value_on >> 8;
  memBlock.bMid = ((s->value_on & 0xFF) << 8) ^ (s->value_off >> 16);
  memBlock.bLow = s->value_off & 0xFFFF;
  memBlock.state = (s->type << 1) ^ s->active;
  EEPROM.put(address, memBlock);
}

bool RemoteController::RFAvailable() {
    return receiver->available();
}

void RemoteController::resetRFAvailable() {
    receiver->resetAvailable();
}

unsigned long RemoteController::getReceivedRFValue() {
    unsigned long currentMillis = millis();
    unsigned long value = receiver->getReceivedValue();
    if (value != lastValue || currentMillis - lastTimestamp >= 1000) {
      lastValue = value;
      lastTimestamp = currentMillis;
      return value;
    }

    return 0;
    //processOwnOrExternalSignal(value);
    //Serial.print("RECEIVED ");
    //Serial.println(value);    
}

void RemoteController::processOwnOrExternalSignal(unsigned long value) {
    bool found;
    for (int i = 0; i < MAX_SWITCHES; i++) {
        switch_t *s = getSwitch(i);
        if (s->value_on == value) {
            found = (s->type == SWITCH_TYPE_TOGGLE) || !s->active;
            s->active = (s->type == SWITCH_TYPE_TOGGLE)? !s->active : true;
        }
        if (found) {
            Serial.print("RECEIVED ");
            Serial.println(value);    
            Serial.print("STATUS ");
            Serial.println(i);
            Serial.println(s->active);
            found = false;
            this->saveSwitch(i);
            continue;
        }

        if (s->value_off == value && s->type == SWITCH_TYPE_ONOFF) {
            found = s->active;
            s->active = false;
        }
        if (found) {
            Serial.print("RECEIVED ");
            Serial.println(value);    
            Serial.print("STATUS ");
            Serial.println(i);
            Serial.println(s->active);
            found = false;
            this->saveSwitch(i);
        }
    }
};

void RemoteController::sendRF(unsigned long code) {
    transmitter->send( code, 24);
    delay(100);
    //processOwnOrExternalSignal(code);
}

void RemoteController::switchById(int device_id, int onoff) {
    receiver->disableReceive();
    if (onoff) {
      transmitter->switchOn( 1, device_id);
    } else {
      transmitter->switchOff( 1, device_id);      
    }
    delay(400);
    receiver->enableReceive(RF_RX_PIN);
    transmitter->enableTransmit(RF_TX_PIN);
    transmitter->disableReceive();
    
    Serial.print("SWITCH ");
    Serial.println(device_id);
    //processOwnOrExternalSignal(code);
}
