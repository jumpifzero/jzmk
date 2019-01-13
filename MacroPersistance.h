#ifndef MACROPERSISTANCE_H
#define MACROPERSISTANCE_H

#include <Arduino.h>

void writeToEEPROM(byte* data, int len);
void readFromEEPROM(byte* dest, int bufSize);

#endif
