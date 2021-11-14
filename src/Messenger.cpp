//ADDED FOR COMPATIBILITY WITH WIRING
extern "C"
{
#include <stdlib.h>
}
#include "Messenger.h"

Messenger::Messenger()
{
  init(' ');
}

Messenger::Messenger(char separator)
{
  if (separator == 10 || separator == 13 || separator == 0)
    separator = 32;
  init(separator);
}

void Messenger::init(char separator)
{
  callback = NULL;
  cmdlist = NULL;
  token[0] = separator;
  token[1] = 0;
  bufferLength = MESSENGERBUFFERSIZE;
  bufferLastIndex = MESSENGERBUFFERSIZE - 1;
  reset();
}

void Messenger::attach(messengerCallbackFunction newFunction)
{
  callback = newFunction;
}

void Messenger::attachCommands(cmdlist_t* commands)
{
  cmdlist = commands;
}

void Messenger::reset()
{
  bufferIndex = 0;
  messageState = 0;
  current = NULL;
  last = NULL;
  dumped = 1;
}

int Messenger::readInt()
{

  if (next())
  {
    dumped = 1;
    return atoi(current);
  }
  return 0;
}

// Added based on a suggestion by G. Paolo Sanino
long Messenger::readLong()
{

  if (next())
  {
    dumped = 1;
    return atol(current); // atol for long instead of atoi for int variables
  }
  return 0;
}

char Messenger::readChar()
{

  if (next())
  {
    dumped = 1;
    return current[0];
  }
  return 0;
}

double Messenger::readDouble()
{
  if (next())
  {
    dumped = 1;
    return atof(current);
  }
  return 0;
}

void Messenger::copyString(char *string, uint8_t size)
{

  if (next())
  {
    dumped = 1;
    strlcpy(string, current, size);
  }
  else
  {
    if (size)
      string[0] = '\0';
  }
}

uint8_t Messenger::checkString(char *string)
{

  if (next())
  {
    if (strcmp(string, current) == 0)
    {
      dumped = 1;
      return 1;
    }
    else
    {
      return 0;
    }
  }
  return 0;
}

uint8_t Messenger::next()
{

  char *temppointer = NULL;
  switch (messageState)
  {
  case 0:
    return 0;
  case 1:
    temppointer = buffer;
    messageState = 2;
  default:
    if (dumped)
      current = strtok_r(temppointer, token, &last);
    if (current != NULL)
    {
      dumped = 0;
      return 1;
    }
  }

  return 0;
}

uint8_t Messenger::available()
{

  return next();
}

uint8_t Messenger::process(int serialByte)
{
  messageState = 0;
  if (serialByte > 0)
  {

    switch (serialByte)
    {
    case 0:
      break;
    case 13: // LF
      break;
    case 10: // CR
      buffer[bufferIndex] = 0;
      reset();
      messageState = 1;
      current = buffer;
      break;
    default:
      buffer[bufferIndex] = serialByte;
      bufferIndex++;
      if (bufferIndex >= bufferLastIndex)
        reset();
    }
  }
  if (messageState == 1) {
    if (callback != NULL) {
      (*callback)();
    } else if (cmdlist != NULL) {
      messageCompleted();
    }
  }
  return messageState;
}

void Messenger::handleParams(char *param, const char sep) {
  byte count = 0;
  args[0] = NULL;
  while (*++param && count < MAX_ARGS) {
    while (*param == sep) {
      param++;
    }
    args[count] = param;
    args[++count] = NULL;
    while (*++param && *param != sep) { }
    *param = '\0';
  }
}

int Messenger::handleData(char cmdline[]) {
  cmdlist_t *cp = cmdlist;
  while (cp->commandname) {
    if (strcasestr(cmdline, cp->commandname) == cmdline) {
      handleParams(cmdline + strlen(cp->commandname), ' ');
      cp->commandfunc(args);
      return 1;
    }
    cp++;
  }
  Serial.print(cmdline);
  Serial.println("not found");
  return 0;
}

// Define messenger function
messengerCallbackFunction Messenger::messageCompleted() {
  // This loop will echo each element of the message separately
  while ( available() ) {
    if (next())
    {
      dumped = 1;
      handleData(current);
    }
  }
  return 0;
}
