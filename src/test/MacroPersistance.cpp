#include "MacroPersistance.h"

#include <EEPROM.h>

// Writes an int to eeprom. An int occupies 2 bytes.
void writeIntToEEPROM(int address, int value) {
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);
  
  EEPROM.write(address, two);
  EEPROM.write(address + 1, one);
}

// Reads an int from eeprom.
int readIntFromEEPROM(int address) {
  long two = EEPROM.read(address);
  long one = EEPROM.read(address + 1);
 
  return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}

void writeToEEPROM(byte* data, int len) { 
  int actualLen;
  actualLen = min(len, EEPROM_SIZE);
  // write the data
  for(int i=0 ; i<actualLen ; i++) { 
    EEPROM.write(i, data[i]);
  }
}

void readFromEEPROM(byte* dest, int bufSize) { 
  byte value;
  int actualLen = min(bufSize, EEPROM_SIZE);
  for(int i=0 ; i<actualLen ; i++) { 
    value = EEPROM.read(i);
    dest[i] = value;
  } 
}
