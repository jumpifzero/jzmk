#include "MacroPersistance.h"
#include <EEPROM.h>

void writeToEEPROM(byte* data, int len) { 
  int actualLen;
  actualLen = min(len, EEPROM.length());
  for(int i=0 ; i<actualLen ; i++) { 
    EEPROM.write(i, data[i]);
  }
}

void readFromEEPROM(byte* dest, int bufSize) { 
  byte value;
  int actualLen = min(bufSize, EEPROM.length());
  for(int i=0 ; i<actualLen ; i++) { 
    value = EEPROM.read(i);
    dest[i] = value;
  } 
}
