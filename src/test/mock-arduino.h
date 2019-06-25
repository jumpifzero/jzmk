#include <cstddef>
#include <cstdint>
#include <stdio.h>

class Keyboard_ {
 public:
  Keyboard_(void);
  void begin();
  void end(void);
  size_t write(uint8_t k);
  size_t write(const uint8_t *buffer, size_t size);
  size_t press(uint8_t k);
  size_t release(uint8_t k);
  void releaseAll(void);
};
extern Keyboard_ Keyboard;

class Serial_ {
 public:
  static void println();
  static void println(byte);
  static void println(const char*);
  static void print(int);
  static void print(const char[]);
  static void begin(int);
  static void end();

};

extern Serial_ Serial;
