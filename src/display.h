#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>
#include <Arduino.h>
#include "LCDIC2.h"

typedef struct lcdline_s {
  String buffer;
  unsigned int pos;
  unsigned int length;
} lcdline_t;

typedef struct displaydata_s
{
  lcdline_t line[2];
} displaydata_t;

class Display
{

public:
 Display(LCDIC2* lcd);
 void setLine(const char*string, unsigned int line);
 void update();
private:
 displaydata_t _data;
 LCDIC2* _lcd;
};

#endif
