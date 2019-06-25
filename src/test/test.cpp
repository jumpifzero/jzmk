#include <iostream>
#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <assert.h>
#include "../../macrotypes.h"

using namespace std;

#define MACROSIZE 11
#define ACTIONSMEMSIZE 500

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

extern KBState kbState;

extern void shiftMacroActions(macro* macros, byte firstMacroToShift, int positions);

extern void kbStateInit();

extern void record(byte row, byte column);

void sendOrRecordKeyPress(byte row, byte column);

void sendOrRecordKeyRelease(byte row, byte column);

void macroKeyPress(byte row, byte column);

// ------------------------------------------------------------
// Functions to be mocked
void writeToEEPROM(unsigned char*, int){}

void readFromEEPROM(unsigned char*, int){}

void digitalRead(unsigned char){}

void digitalWrite(unsigned char, unsigned char){}

// Simulates press and release of a key
void tap(byte row, byte column) {
  sendOrRecordKeyPress(row, column); // L
  sendOrRecordKeyRelease(row, column);
}

// ------------------------------------------------------------
// TESTS

// Tests that we can enter and exit recording mode
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


// Tests that we can record keys as they are pressed and assign to a macro
void testRecordingSingle(){
  // confirm it is not in recording mode
  assert(!kbState.recording);
  record(0,0); // now it is recording. Confirm it
  assert(kbState.recording);
  // press and release a few keys
  tap(4,6); // H
  tap(3,3); // E
  tap(4,9); // L
  tap(4,9); // L
  tap(3,9); // O
  macroKeyPress(0,1); // Save recording in macro M1
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
  tap(4,5); // G
  tap(3,2); // W
  tap(4,8); // K
  tap(4,8); // K
  tap(3,8); // I
  tap(3,10); // P
  macroKeyPress(0,1); // Save recording in macro M1
  assert(kbState.actionsLen == 12);
  assert(kbState.actions[0].key == 'g');
  assert(kbState.actions[2].key == 'w');
  assert(kbState.actions[10].key == 'p');
}

int main() {
  // initialize the kbState global structure
  kbStateInit();
  //testRecord();
  testRecordingSingle();
  //  shiftMacroActions(m, 0, 2);
  printf("Tests passed.\n");
  exit(0);
}
