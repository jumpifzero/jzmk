#ifndef MACROPERSISTANCE_H
#define MACROPERSISTANCE_H
#include <Arduino.h>

#define EEPROM_SIZE 1024

void writeToEEPROM(byte* data, int len);
void readFromEEPROM(byte* dest, int bufSize);

#endif
