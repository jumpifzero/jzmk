#ifndef ARDUINO_H
#define ARDUINO_H

#include <cstdint>

typedef uint8_t byte;

#define HIGH 0x1 
#define LOW  0x0

#define LSBFIRST 0
#define MSBFIRST 1

int digitalRead(uint8_t);

#endif
