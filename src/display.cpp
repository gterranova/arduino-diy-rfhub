//ADDED FOR COMPATIBILITY WITH WIRING
#include "display.h"
#include "LCDIC2.h"

Display::Display(LCDIC2* lcd)
{
    _lcd = lcd;
}

void Display::setLine(const char* string, unsigned int line)
{
    _data.line[line].buffer = string;
    _data.line[line].pos = 0;
    _data.line[line].length = _data.line[line].buffer.length();
}

void Display::update()
{
    static unsigned long timer = 0;
    unsigned long interval = 1000;
    if(millis() - timer > interval)
    {
        _lcd->clear();
        timer = millis();
        String line;
        for (int i=0; i<2; i++) {
            line = _data.line[i].buffer.substring(_data.line[i].pos, _data.line[i].pos+16);
            int tailIndex = 16 - line.length() - 1;
            if (tailIndex>0 && _data.line[i].length > 16) {
                line += " "+ _data.line[i].buffer.substring(0, tailIndex);
            }

            //Serial.print("line2: ");Serial.print(_data.line[i].pos);Serial.print(": "); Serial.println(line);
            _lcd->setCursor(0,i);
            _lcd->print(line);

            if (_data.line[i].length > 16) {
                _data.line[i].pos += 1;
            }
            if (_data.line[i].pos == _data.line[i].length) {
                _data.line[i].pos = 0;
            };
        }
    }
}
