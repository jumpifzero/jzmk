/*
 
 Firmware for a keyboard based on an ATMega32u4.
 TKL GB Layout with an extra row above the Fs 
 for macro recording and extra keys.
 
 Copyright: Tiago Almeida (jumpifzero@gmail.com)
 All rights reserved.
 2019
 
*/

#include <Keyboard.h>
#include "MacroPersistance.h"

// Rows and Coumns pin definitions
#define MAXSR 3 // Number of cascaded shift registers for the columns
#define COLUMNCOUNT 17 // Number of columns connected to the shift registers
#define ROWCOUNT 7 // Number of rows connected to the arduino. The code assumes maximum is 8 ! 
#define SRMINDELAY 1 // minimum time for an SR input

#define KEY_F13       0xF0
#define KEY_F14       0xF1
#define KEY_F15       0xF2
#define KEY_F16       0xF3
#define KEY_F17       0xF4
#define KEY_F18       0xF5
#define KEY_F19       0xF6
#define KEY_F20       0xF7
#define KEY_F21       0xF8
#define KEY_F22       0xF9
#define KEY_F23       0xFA
#define KEY_F24       0xFB

// Rows are directly connected
#define ROW0PIN 2
#define ROW1PIN 3
#define ROW2PIN 4
#define ROW3PIN 5
#define ROW4PIN 6
#define ROW5PIN 7
#define ROW6PIN 8

// Columns are connected through shift registers
#define COLSRDATA 14  // push the bit to this pin
#define COLSRCLOCK 10 // pulse this pin L-H-L to shift in the data
#define COLSRLATCH 16  // pulse this pin L-H-L to move the data shifted in to the output.

#define SCANSTOP 12 // normally off button, normally low, that when goes high stops scan.
#define SCANSTOPPED 13 // LED, goess high when the scan is stopped.

#define ACTION_PRESS 1
#define ACTION_RELEASE 2
#define ACTIONSMEMSIZE 500 // Total memory for macros in pairs action+key.
#define MACROSIZE 11       // Total number of dynamic macros

typedef struct action_s { 
  byte action;
  byte key;  
} action;

typedef struct macro_s { 
  byte len;
  action* actions;  
} macro;



// global constants
// ----------------
const byte ROWSPINS[7] = {ROW0PIN,ROW1PIN,ROW2PIN,ROW3PIN,ROW4PIN,ROW5PIN,ROW6PIN}; // The pins used to connect to each row.
#define NON 0

#define KTODO KEY_LEFT_CTRL
// Note on the below keycodes:
// The keyboard.press method accepts an ascii code, not an HID keycode.
// This makes it easier to send specific letters but complicated to send special keys (like the windows key).
// According to this article http://joshfire-tech.tumblr.com/post/65032568887/arduino-keyboard-emulation-send-real-key-codes
// (which is copied below in case the link dies), there is a trick which is to add 136 to the code. This will work but 
// limits the keycodes that can be sent to the ones below 119.
//size_t Keyboard_::press(uint8_t k) 
//{
//        uint8_t i;
//        if (k >= 136) {  // it's a non-printing key (not a modifier)
//                k = k - 136;
//        } else if (k >= 128) {  // it's a modifier key
//                _keyReport.modifiers |= (1<<(k-128));
//                k = 0;
//        } else {  // it's a printing key
//                k = pgm_read_byte(_asciimap + k);
//                // ... more code ...
//        }
//        // ... more code ...
//        // k is the key code sent to the computer
//        return 1;
//}
//If k is < 128, it’s an ascii character, if it’s between 128 and 136 it’s a modifier (shift, alt, ctrl…) and if it’s greater than 136, it’s an unknown key.
//If you call Keyboard.press(238), k is greater than 136 and is transformed into 238 - 136 = 102 = 0x66 = “power” key! That’s it! To send a key code instead of an ascii character, just add 136 to it.
//Since k is a uint8_t, the max value is 255 so you are limited to key codes < 119.

const byte KEYMAP[ROWCOUNT][COLUMNCOUNT] = {
  {NON, KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24, NON, NON, NON, NON},
  {KEY_ESC, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, NON, 0xCE/*Print 0x46+0x88*/, 0xCF, 0xD0}, /*these codes have 0x88 added*/
  {'`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', KEY_BACKSPACE, KEY_INSERT, KEY_HOME, KEY_PAGE_UP},
  {KEY_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', KEY_RETURN, KEY_DELETE, KEY_END, KEY_PAGE_DOWN },
  {KEY_CAPS_LOCK, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 0xBA, NON, '(', '{', '['},
  {KEY_LEFT_SHIFT, 0xEC/*0x64+0x88*/, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KEY_RIGHT_SHIFT, NON, '\'', KEY_UP_ARROW, 0xBC /*+0x88*/},
  {KEY_LEFT_CTRL, 0x83, KEY_LEFT_ALT, NON, NON, NON, ' ', NON, NON, NON, KEY_RIGHT_ALT, 0x87, 0xED, KEY_RIGHT_CTRL, KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW}
};


// =============================================
// global variables
// =============================================
// Stores the current state of the matrix, as columns,
// each bit of the byte representing state of one switch.
// This is the state already debounced.
// Check functions matrixStateInit, matrixStateGet, matrixStateSet to manipulate this.
byte matrixState[COLUMNCOUNT];


// Stores a byte per switch that is used to collect up to the last 8
// readings of the switch, to allow for a simple debounce mechanism.
// See matrixReadingsInit
byte matrixReadings[ROWCOUNT][COLUMNCOUNT];


// Global keyboard state
struct kbState_s { 
  byte recording;
  action currentRecording[200];
  int currentRecordingLen;
  int currentRecordingBufSize;
  macro macros[MACROSIZE];     // The macros mapped by each macro key m1,m2,etc.
  action actions[ACTIONSMEMSIZE];  // The actions comprising each macro, pointed by macro
  // the actions array is _always_ kept compacted from index 0 up to actionsLen.
  int actionsLen;
} kbState;


/**
 * Sets state of all switches in the matrix as not-pressed (LOW)
 **/
void matrixStateInit() {
  for(byte column=0 ; column<COLUMNCOUNT ; column++) {
    matrixState[column] = 0;
  }
}


// Dumps all macros into EEPROM.
void writeMacrosToEEPROM() { 
  // All info we need to dump is on kbState. 
  // First we need to dump macros[]
  // then actions[].
  int macroArraySize = sizeof(macro) * MACROSIZE;
  int actionsArraySize = sizeof(action) * ACTIONSMEMSIZE;
  writeToEEPROM((byte*)kbState.macros, macroArraySize+actionsArraySize);
}


void readMacrosFromEEPROM() { 
  int macroArraySize = sizeof(macro) * 12;
  int actionsArraySize = sizeof(action) * ACTIONSMEMSIZE;
  readFromEEPROM((byte*)kbState.macros, macroArraySize+actionsArraySize);
}


void kbStateInit() { 
  kbState.recording = false;  
  kbState.currentRecordingLen = 0;
  kbState.currentRecordingBufSize = 256;
  //readMacrosFromEEPROM();
  // by default the eeprom is all 0xFF. We look at the len of 
  // macros[0] to see if what was loaded makes sense or we need to 
  // initialize to a sensible initial state.
  if (kbState.macros[0].len == 255) {
    kbState.actionsLen = 0;
    for(int i=0 ; i<12 ; i++){
      kbState.macros[i].len = 0;  
    }
    //writeMacrosToEEPROM();
  }
}


void currentRecordingAddKeyPress(byte key) {
  // Serial.print("reclen="); Serial.print(kbState.currentRecordingLen); Serial.println();
  kbState.currentRecording[kbState.currentRecordingLen].action = ACTION_PRESS;
  kbState.currentRecording[kbState.currentRecordingLen].key = key;
  kbState.currentRecordingLen = kbState.currentRecordingLen + 1;
}


void currentRecordingAddKeyRelease(byte key) {
  // Serial.print("reclen="); Serial.print(kbState.currentRecordingLen); Serial.println();
  kbState.currentRecording[kbState.currentRecordingLen].action = ACTION_RELEASE;
  kbState.currentRecording[kbState.currentRecordingLen].key = key;
  kbState.currentRecordingLen = kbState.currentRecordingLen + 1;
}


void currentRecordingClear(){
  kbState.currentRecordingLen = 0;
}


/**
 * Returns the state of the switch at row, column, as a bool.
 * Assume false is not pressed, anything else means pressed.
 * Rows are numbered bottom-up with row 0 being the row with the space bar.
 **/
bool matrixStateGet(byte row, byte column) {
  byte columnState = matrixState[column];
  // mask the column state, depending on the row.
  // row0 -> mask with 0x01 (2^0)
  // row1 -> mask with 0x02
  // row2 -> mask with 0x04
  // row3 -> mask with 0x08
  // row4 -> mask with 0x10
  // row5 -> mask with 0x20
  // row6 -> mask with 0x40
  // row7 -> mask with 0x80 (2^7)
  byte bitmask = (((byte)0x1) << row);
  return (bool)(columnState & bitmask);
}


/**
 * Records state of switch in row row and column column as a bit.
 **/
void matrixStateSet(byte row, byte column, bool state) {
  byte columnState = matrixState[column];
  byte bitmask = 0;
  if ( state ) { // if setting that bit as 1, we need to
    // or with as mask full of zeros except in that position.
    bitmask = 0x1 << row;
    columnState = columnState | bitmask;
  } else {
    // when setting that bit as 0, and with 1's everywhere
    // except that position.
    bitmask = 0xFE; // 11111110
    for(int i=0; i<row; i++){
      bitmask = ((bitmask << 1) | 0x1); // inject 1 on the right  
    }
    columnState &= bitmask;
  }
  matrixState[column] = columnState;
}


void matrixReadingsInit() {
  for(byte row=0 ; row<ROWCOUNT ; row++) {
    for(byte column=0 ; column<COLUMNCOUNT ; column++) {
      matrixReadings[row][column] = 0xFF; // An open switch reads as a HIGH (1)
    }
  }
}


byte matrixReadingsGet(byte row, byte column) {
  return matrixReadings[row][column];
}


/**
 * Pushes a new bit as a reading for the switch in row/column
 * LSB is the most recent push.
 * Returns a byte representing the last 8 reads of that switch.
 * A 0x0 returned here means at least 8 readings with the switch pressed.
 **/
byte matrixReadingsPush(byte row, byte column, bool state) {
  byte readings = matrixReadings[row][column];
  readings = (readings << 1);
  if(state) {
    readings |= 0x1; // inject a 1
  }
  matrixReadings[row][column] = readings;
  return readings;
}


/**
 * Sets all columns connected to the shift registers to be High
 **/
void disableAllColumns() {
  digitalWrite(COLSRLATCH, LOW);
  for(int i=0; i<MAXSR; i++) {
    shiftOut(COLSRDATA, COLSRCLOCK, LSBFIRST, 0xFF); // shift out a High
  }
  digitalWrite(COLSRLATCH, HIGH);
  digitalWrite(COLSRLATCH, LOW);
}


/**
 * Selects the first column by setting it low.
 * Returns 0. The selected column.
 **/
byte selectFirstColumn() {
  digitalWrite(COLSRLATCH, LOW);
  digitalWrite(COLSRCLOCK, LOW);
  digitalWrite(COLSRDATA,0);
  //delay(SRMINDELAY);
  digitalWrite(COLSRCLOCK,HIGH);
  //delay(SRMINDELAY);
  digitalWrite(COLSRCLOCK,LOW);

  digitalWrite(COLSRLATCH, HIGH);
  digitalWrite(COLSRLATCH, LOW);
  return 0;
}

/**
 * Selects the next column.
 * Returns the index of the selected column between [0,numColumns[
 **/
byte selectNextColumn(byte currentColumn, byte numColumns) {
  if ( currentColumn >= (numColumns-1) ) { 
    disableAllColumns();
    return selectFirstColumn();
  } else { 
    digitalWrite(COLSRLATCH, LOW);
    digitalWrite(COLSRDATA, HIGH);
    digitalWrite(COLSRCLOCK, HIGH);
    //delay(SRMINDELAY);
    digitalWrite(COLSRCLOCK, LOW);
    digitalWrite(COLSRDATA, LOW);
    digitalWrite(COLSRLATCH, HIGH);
    digitalWrite(COLSRLATCH, LOW);
    return (currentColumn + 1) % numColumns;
  }
}

/**
 * Outputs the status of the rows via rows array.
 * numRows can be up to 8.
 * Returns a bitmap for fast decision on the status of the rows.
 * Will return 0xFF if nothing is pressed.
 **/
byte readRows(bool rows[], byte numRows, byte rowsPins[]) {
  byte result = 0xFF;
  byte pinValue = 1; // pins will be 1 unless the key is pressed
  for(byte i=0 ; i<numRows ; i++) {
    pinValue = digitalRead(rowsPins[i]);
    result = (result << 1) | (pinValue & 1);
    rows[i] = pinValue;
  }
  return result;
}

void setup() { 
  pinMode(ROW0PIN, INPUT_PULLUP);
  pinMode(ROW1PIN, INPUT_PULLUP);
  pinMode(ROW2PIN, INPUT_PULLUP);
  pinMode(ROW3PIN, INPUT_PULLUP);
  pinMode(ROW4PIN, INPUT_PULLUP);
  pinMode(ROW5PIN, INPUT_PULLUP);
  pinMode(ROW6PIN, INPUT_PULLUP);

  pinMode(SCANSTOPPED, OUTPUT);
  digitalWrite(SCANSTOPPED, HIGH);
  pinMode(SCANSTOP, INPUT);
  
  pinMode(COLSRDATA, OUTPUT);
  pinMode(COLSRCLOCK, OUTPUT);
  pinMode(COLSRLATCH, OUTPUT);
  
  // set all outputs in the shift register as high
  disableAllColumns();
  
  Serial.begin(9600);
  Keyboard.begin();

  kbStateInit();
}


// ===============================================
// EXECUTORS. These functions carry out use functions.
void sendKeyPress(byte row, byte column) {
  Keyboard.press(KEYMAP[row][column]);
}


void sendKeyRelease(byte row, byte column) {
  Keyboard.release(KEYMAP[row][column]);
}


void recordKeyPress(byte row, byte column) { 
  currentRecordingAddKeyPress(KEYMAP[row][column]);
}


void recordKeyRelease(byte row, byte column) { 
  currentRecordingAddKeyRelease(KEYMAP[row][column]);
}


void sendOrRecordKeyPress(byte row, byte column) { 
  if(!kbState.recording){ return sendKeyPress(row, column); }
  //Serial.println("Key Pressed while recording");
  return recordKeyPress(row, column);  
}


void sendOrRecordKeyRelease(byte row, byte column) { 
  if(!kbState.recording){ 
    return sendKeyRelease(row, column); 
  }
  return recordKeyRelease(row, column);  
}


void nop(byte row, byte column) { }


void record(byte row, byte column) { 
  // Will start or stop the recording
  kbState.recording = !kbState.recording;
  if(kbState.recording){
    Serial.println("Recording started");
  } else { 
    Serial.println("Recording stopped");  
  }
}


void executeActions(action* acts, int len) {
  action currentAction; 
  for(int i=0 ; i< len ; i++){
    currentAction = acts[i];
    if ( currentAction.action == ACTION_PRESS ) { 
      Keyboard.press(currentAction.key);    
    } else if ( currentAction.action == ACTION_RELEASE ) { 
      Keyboard.release(currentAction.key);
    }
  }    
}


/**
 * Shifts an actions array inplace to the right, right to left.
 */
void shiftActionsRight(action* actions, int actionsLen, int positions) {
  for(int i=actionsLen-1 ; i>=0 ; i--) {
    actions[i+positions] = actions[i];
  }    
}


/**
 * Shifts an actions array inpace to the left. Left to right.
 * positions is negative
 */
void shiftActionsLeft(action* actions, int actionsLen, int positions) { 
  for(int i=0 ; i<actionsLen ; i++) {
    actions[i+positions] = actions[i];
  }
}


/**
 * 
 */
void shiftMacroActions(macro* macros, byte firstMacroToShift, int positions) { 
  // first determine the lastMacro with a non 0 length.
  byte lastMacroToShift;
  for(byte i=firstMacroToShift ; i<MACROSIZE ; i++) { 
    if(macros[i].len > 0){
      lastMacroToShift = i;  
    }
  }
  // if we are shifting right we need to start from the the right, 
  // else we start from the left.
  if(positions>0) {
    // shift the macros to the right
    for(int i=lastMacroToShift ; i>firstMacroToShift ; i--) { 
        shiftActionsRight(macros[i].actions, macros[i].len, positions);
        macros[i].actions = macros[i].actions + positions;
    }  
  } else { 
     for(int i=firstMacroToShift ; i<lastMacroToShift ; i++) { 
        shiftActionsLeft(macros[i].actions, macros[i].len, positions);
        macros[i].actions = macros[i].actions + positions;
    }
  } 
}


int addLengthOfMacroActions(macro* macros, byte startIndex, byte endIndex) {
  int result = 0;
  for(int i=startIndex ; i<endIndex ; i++) {
    result += macros[i].len;
  }
  return result;
}


/**
 * 
 */
void macrosMakeSpaceForRecording(byte macroIndex, byte numberActionsRecorded) { 
  byte totalMacros = MACROSIZE;
  int location = kbState.actionsLen;
  int firstEmptyIndex;
  action* nextMacroActionsPtr = NULL;

  if ( macroIndex == (totalMacros - 1)) {
    // Last macro always has space. There's nothing to do.
    return;
  }

  // add the size of all the macros to the left and right of macroIndex
  int totalSizeToTheLeft = 0;
  int totalSizeToTheRight = 0;
  totalSizeToTheRight = addLengthOfMacroActions(kbState.macros, macroIndex+1, totalMacros);
  totalSizeToTheLeft = addLengthOfMacroActions(kbState.macros, 0, macroIndex);
  int startMacroIndex = totalSizeToTheLeft + 1;
  int endMacroIndex = startMacroIndex + numberActionsRecorded;
  // the macros to the right may need to be shifted to the right
  // if this one is larger than it was before
  // or to the left if this one is smaller than was before.
  int positionsToShiftRight = ( numberActionsRecorded - kbState.macros[macroIndex].len);
  // positive -> shift macros to the right, negative -> shift them to the left.
  shiftMacroActions(kbState.macros, macroIndex+1, positionsToShiftRight);
}


/**
 * Purpose of this function is to send the whole content
 * of the macros memory. Useful for debugging.
 */
void dumpMacros(byte row, byte column) { 
  char charVal[4];

  for(int i=0 ; i<MACROSIZE ; i++){
    macro m = kbState.macros[i];
    Serial.println();
    Serial.print("macro ");
    Serial.print(i);
    Serial.print("length ");
    Serial.print(m.len);
    Serial.println();
    for(int j=0 ; j<m.len ; j++){
      sprintf(charVal, "%02X", m.actions[j]);
      Serial.print(charVal); Serial.print(" ");  
    }
  }
}


/**
 * Called when one of the Macro keys is pressed.
 */
void macroKeyPress(byte row, byte column){
  if ( kbState.recording ) { // record a new macro
    // The keys that were captured are in currentRecording. 
    // The size of the recording is in currentRecordingLen.
    kbState.macros[column-1].len = kbState.currentRecordingLen;
    // All the actions of all macros need to be compacted on
    // a single array. So if we are trying to overwrite the 
    // Macro N we need to make sure there's no space after
    // the actions of N. This means we may need to shift left
    // all macros above N or shift them right.
    // This is dealt on function macrosMakeSpaceForRecording
    macrosMakeSpaceForRecording(column-1, kbState.currentRecordingLen);
    // Add the length of all the macro actions to the left of this one.

    int startOfAction = kbState.actionsLen;
    for(int i=0 ; i<kbState.currentRecordingLen; i++){
      kbState.actions[kbState.actionsLen].action = kbState.currentRecording[i].action;
      kbState.actions[kbState.actionsLen].key = kbState.currentRecording[i].key;  
      kbState.actionsLen = kbState.actionsLen + 1;
    }
    kbState.macros[column-1].actions = &(kbState.actions[startOfAction]);
    currentRecordingClear();
    kbState.recording = false;
    Serial.println("Recording stopped");
    Serial.print("Saved macro ");
    Serial.println(column);
    // TODO store all macros into eeprom
    
  } else { // execute the macro
    Serial.println("Executing macro");
    executeActions(kbState.macros[column-1].actions, kbState.macros[column-1].len);    
    Serial.println("Finished executing macro");
  }
}


// Fixed macro definitions
action TYPE_PARENS[6] = {
  {ACTION_PRESS, '('}, 
  {ACTION_RELEASE, '('}, 
  {ACTION_PRESS, ')'}, 
  {ACTION_RELEASE, ')'}, 
  {ACTION_PRESS, KEY_LEFT_ARROW}, 
  {ACTION_RELEASE, KEY_LEFT_ARROW}
};
macro MACRO_OPEN_CLOSE_PARENS = {6, TYPE_PARENS};

action TYPE_CURLYS[6] = {
  {ACTION_PRESS, '{'}, 
  {ACTION_RELEASE, '{'}, 
  {ACTION_PRESS, '}'}, 
  {ACTION_RELEASE, '}'}, 
  {ACTION_PRESS, KEY_LEFT_ARROW}, 
  {ACTION_RELEASE, KEY_LEFT_ARROW}
};
macro MACRO_OPEN_CLOSE_CURLYS = {6, TYPE_CURLYS};

action TYPE_SQBRACKETS[6] = {
  {ACTION_PRESS, '['}, 
  {ACTION_RELEASE, '['}, 
  {ACTION_PRESS, ']'}, 
  {ACTION_RELEASE, ']'}, 
  {ACTION_PRESS, KEY_LEFT_ARROW}, 
  {ACTION_RELEASE, KEY_LEFT_ARROW}
};
macro MACRO_OPEN_CLOSE_SQBRACKETS = {6, TYPE_SQBRACKETS};

void simulateTypingSimple(byte row, byte column){
  Keyboard.press('(');
  Keyboard.release('(');
  Keyboard.press(')');
  Keyboard.release(')');
  Keyboard.press(KEY_LEFT_ARROW);
  Keyboard.release(KEY_LEFT_ARROW);
    
}


/*
 * This function executes macros. There are 5 fixed macros.
 */
void simulateTyping(byte row, byte column){
  action* actions;
  macro m;
  if(row==4 && column==14){
    m = MACRO_OPEN_CLOSE_PARENS;
  } else if (row==4 && column==15) { 
    m = MACRO_OPEN_CLOSE_CURLYS;
  } else if (row==4 && column==16) {
    m = MACRO_OPEN_CLOSE_SQBRACKETS; 
  } else { 
    return;  
  } 
  actions = m.actions;
  executeActions(actions, m.len);
} 


// Action keymap. For every key, maps a function which determines what happens.
const void (*ACTIONS[ROWCOUNT][COLUMNCOUNT]) (byte row, byte column) = {
  {sendOrRecordKeyPress, macroKeyPress, macroKeyPress, macroKeyPress, macroKeyPress, macroKeyPress, macroKeyPress, macroKeyPress, macroKeyPress, macroKeyPress, macroKeyPress, macroKeyPress, dumpMacros, record, nop, nop, nop},
  {sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress},
  {sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress},
  {sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress},
  {sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, simulateTyping, simulateTyping, simulateTyping},
  {sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress},
  {sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress, sendOrRecordKeyPress}
};


void onPressDispatchAction(byte row, byte column){
  return (ACTIONS[row][column])(row, column);
}


void onReleaseDispatchAction(byte row, byte column){
  if(row>0){
    return sendOrRecordKeyRelease(row, column);  
  }  
}


#define MAX_MULTI_KEY 3

//typedef struct MultiKey_s {
//   byte len;
//   byte keys[MAX_MULTI_KEY];
//} MultiKey;


void debugPrintRowCol(byte row, byte col) { 
  Serial.print(row);
  Serial.print(",");
  Serial.print(col);
  Serial.println();   
}

void pressReleaseMultiple(byte row, byte column){
    
}

void scanMatrix (int selectedColumn) {
  bool rows[7];
  
  byte columnBitmap = 0xFF;
  bool switchOpen = true;
  byte last8Readings = 0xFF;
  bool switchState = false;

  columnBitmap = readRows(rows, 7, ROWSPINS);
  // check each switch of this column
  for(byte row=0; row<ROWCOUNT; row++) {
    // Read switch on row row, column selectedColumn.
    switchOpen = (bool)digitalRead(ROWSPINS[row]);
    
    last8Readings = matrixReadingsPush(row, selectedColumn, switchOpen);
    switchState = matrixStateGet(row, selectedColumn);
    if (last8Readings==0x0 && !switchState) { // stable press, non press to press.
      debugPrintRowCol(row, selectedColumn);
      onPressDispatchAction(row, selectedColumn);
      matrixStateSet(row, selectedColumn, true);

    }else if (last8Readings==0xFF && switchState) { // stable depress, press to non press.
      onReleaseDispatchAction(row, selectedColumn);
      matrixStateSet(row, selectedColumn, false);
    }
  }
  selectedColumn = selectNextColumn(selectedColumn, COLUMNCOUNT);
}


void loop() {
  int selectedColumn = 0;
  
  bool stopScanningOff = true; // used to break out of scanning routine

  disableAllColumns();
  selectedColumn = selectFirstColumn();
  
  while(1) {
    scanMatrix(selectedColumn);
  }

  // We stopped scanning
  Keyboard.releaseAll();
  Keyboard.end();
  //digitalWrite(SCANSTOPPED, LOW);
  Serial.println("Stopped");
  Serial.end();
  while(1){
    delay(500);
  }
}
