#include "Arduino.h"
#include "mock-arduino.h"

void Keyboard_::begin(){}
void Keyboard_::end(){}
size_t Keyboard_::write(uint8_t k){}
size_t Keyboard_::write(const uint8_t *buffer, size_t size){}
size_t Keyboard_::press(uint8_t k){}
size_t Keyboard_::release(uint8_t k){}
void Keyboard_::releaseAll(void){}
Keyboard_::Keyboard_(){}

Keyboard_ Keyboard;

void pinMode(unsigned char, unsigned char){}

void Serial_::println(){}
void Serial_::println(byte){}
void Serial_::println(const char*){}
void Serial_::print(int){}
void Serial_::begin(int){}
void Serial_::end(){}
void Serial_::print(char const*){}

// class Serial_ {
// public:
//   static void println(){}
//   static void println(byte){}
//   static void println(const char*){}
//   static void print(int){}
//   static void print(const char[]){}
//   static void begin(int){}
//   static void end(){}

// };

Serial_ Serial;

void shiftOut(unsigned char, unsigned char, unsigned char, unsigned char){}

//void digitalRead(unsigned char){}

//void digitalWrite(unsigned char, unsigned char){}
