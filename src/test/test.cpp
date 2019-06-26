#include <iostream>
#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <assert.h>
#include "../../macrotypes.h"
#include "mock-arduino.h"

using namespace std;

#define MACROSIZE 11
#define ACTIONSMEMSIZE 500

// Definitions of key coordinates so it is easier to write tests below
#define KEY_M2 0,2
#define KEY_H 4,6
#define KEY_U 3,7
#define KEY_E 3,3
#define KEY_L 4,9
#define KEY_O 3,9
#define KEY_G 4,5
#define KEY_P 3,10
#define KEY_I 3,8
#define KEY_K 4,8
#define KEY_W 3,2

typedef uint8_t byte;

// ------------------------------------------------------------
// Functions to be tested
typedef struct kbState_s { 
  byte recording;
  action currentRecording[200];
  int currentRecordingLen;
  int currentRecordingBufSize;
  macro macros[MACROSIZE];     // The macros mapped by each macro key m1,m2,etc.
  action actions[ACTIONSMEMSIZE];  // The actions comprising each macro, pointed by macro
  // the actions array is _always_ kept compacted from index 0 up to actionsLen.
  int actionsLen;
} KBState;

extern Keyboard_ Keyboard;

extern KBState kbState;

extern void shiftMacroActions(macro* macros, byte firstMacroToShift, int positions);

extern void kbStateInit();

extern void record(byte row, byte column);

void sendOrRecordKeyPress(byte row, byte column);

void sendOrRecordKeyRelease(byte row, byte column);

void macroKeyPress(byte row, byte column);

// ------------------------------------------------------------
// Mocked functions
void writeToEEPROM(unsigned char*, int){}

void readFromEEPROM(unsigned char*, int){}

void digitalRead(unsigned char){}

void digitalWrite(unsigned char, unsigned char){}


// ------------------------------------------------------------
// Helper functions

// Simulates press and release of a key
void tap(byte row, byte column) {
  sendOrRecordKeyPress(row, column); // L
  sendOrRecordKeyRelease(row, column);
}

void storeInMacro(int m){
  macroKeyPress(0,m); // Save recording in macro Mm
}

void tapRecordKey(){
  record(0,0);
}


// ------------------------------------------------------------
// TESTS

// Test that we can enter and exit recording mode
void testRecord(){
  // confirm it is not in recording mode
  assert(!kbState.recording);
  record(0,0);
  // check it is in recording mode
  assert(kbState.recording);
  // disable recording and assert
  record(0,0);
  assert(!kbState.recording);
}

// Test that we can record keys as they are pressed and assign to a macro
void testRecordingM1(){
  // confirm it is not in recording mode
  assert(!kbState.recording);
  record(0,0); // now it is recording. Confirm it
  assert(kbState.recording);
  // press and release a few keys
  tap(KEY_H);
  tap(KEY_E);
  tap(KEY_L);
  tap(KEY_L);
  tap(KEY_O);
  storeInMacro(1);

  // Confirm it is no longer recording
  assert(!kbState.recording);
  // make sure we captured something in the kbState
  printf("%x\n", kbState);
  assert(kbState.actions != NULL);
  assert(kbState.actionsLen == 10);
  assert(kbState.actions[0].key == 'h');
  assert(kbState.actions[2].key == 'e');

  // Now record something longer on top of M1
  record(0,0); // now it is recording. Confirm it
  assert(kbState.recording);
  // press and release a few keys
  tap(KEY_G);
  tap(KEY_W);
  tap(KEY_K);
  tap(KEY_K);
  tap(KEY_I);
  tap(KEY_P);
  storeInMacro(1);

  assert(kbState.actionsLen == 12);
  assert(kbState.actions[0].key == 'g');
  assert(kbState.actions[2].key == 'w');
  assert(kbState.actions[10].key == 'p');

  // Record something shorter on top of M1
  tapRecordKey();
  assert(kbState.recording);
  // press and release a key
  tap(KEY_U);
  storeInMacro(1);

  assert(kbState.actionsLen == 2);
  assert(kbState.actions[0].key == 'u');

}

void testRecordingM2AndReplay(){
  // make sure the kbd is in the state we expect prior to the test
  assert(kbState.actionsLen == 2);

  tapRecordKey();
  tap(KEY_H);
  tap(KEY_E);
  tap(KEY_L);
  tap(KEY_L);
  tap(KEY_O);
  storeInMacro(2);

  assert(kbState.actionsLen == (10+2));
  macroKeyPress(KEY_M2);
  // In theory HELLO was replayed. We can verify looking at Keyboard.history
  assert(Keyboard.history.size() == 10);
}

int main() {
  // initialize the kbState global structure
  kbStateInit();

  testRecord();
  testRecordingM1();
  testRecordingM2AndReplay();

  printf("Tests passed.\n");
  exit(0);
}
