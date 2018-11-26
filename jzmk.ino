/*

*/

#include <Keyboard.h>

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
#define COLSRDATA 11  // push the bit to this pin
#define COLSRCLOCK 9 // pulse this pin L-H-L to shift in the data
#define COLSRLATCH 10  // pulse this pin L-H-L to move the data shifted in to the output.

#define SCANSTOP 12 // normally off button, normally low, that when goes high stops scan.
#define SCANSTOPPED 13 // LED, goess high when the scan is stopped.

// global constants
// ----------------
const byte ROWSPINS[7] = {ROW0PIN,ROW1PIN,ROW2PIN,ROW3PIN,ROW4PIN,ROW5PIN,ROW6PIN}; // The pins used to connect to each row.
// The keymap. GB
#define NON 0
#define KTODO KEY_LEFT_CTRL
const byte KEYMAP[ROWCOUNT][COLUMNCOUNT] = {
  {NON, KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24, NON, NON, NON, NON},
  {KEY_ESC, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, NON, KTODO/*Print*/, KTODO, KTODO}, // TODO
  {KTODO, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', KTODO, KTODO, KTODO, KTODO},
  {KTODO, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', KTODO, KTODO, KTODO, KTODO, KTODO, KTODO },
  {KTODO, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', KTODO, KTODO, KTODO, KTODO, KTODO, KTODO, KTODO},
  {KEY_LEFT_SHIFT, KTODO, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KEY_RIGHT_SHIFT, NON, KEY_UP_ARROW, NON},
  {KEY_LEFT_CTRL, KTODO, KEY_LEFT_ALT, NON, NON, NON, ' ', NON, NON, NON, KEY_RIGHT_ALT, NON, NON, KEY_RIGHT_CTRL, KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW}
  
  
};
#ifdef _smallKeymap
const byte KEYMAP[ROWCOUNT][2] = { 
  {'a', 'b'},  
  {'c', 'd'},
  {'f', 'e'},
  {'g', 'h'},
  {'g', 'h'},
  {'g', 'h'},
  {'g', 'h'}
};
#endif 

// global variables
// ----------------
// Stores the current state of the matrix, as columns,
// each bit of the byte representing state of one switch.
// This is the state already debounced.
// Check functions matrixStateInit, matrixStateGet, matrixStateSet to manipulate this.
byte matrixState[COLUMNCOUNT];

// Stores a byte per switch that is used to collect up to the last 8
// readings of the switch, to allow for a simple debounce mechanism.
// See matrixReadingsInit
byte matrixReadings[ROWCOUNT][COLUMNCOUNT];

/**
 * Sets state of all switches in the matrix as not-pressed (LOW)
 **/
void matrixStateInit() {
  for(byte column=0 ; column<COLUMNCOUNT ; column++) {
    matrixState[column] = 0;
  }
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
    columnState = columnState & bitmask;
  } else {
    // when setting that bit as 0, and with 1's everywhere
    // except that position.
    bitmask = 0xFE; // 11111110
    bitmask = bitmask << row;
    columnState |= bitmask;
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
  delay(SRMINDELAY);
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
  delay(SRMINDELAY);
  digitalWrite(COLSRCLOCK,HIGH);
  delay(SRMINDELAY);
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
    delay(SRMINDELAY);
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
}

//typedef struct KeyQueue_s{
//  byte count = 0;
//  byte pressesToSend[8];
//} KeyQueue;

//void keyQueueInsert(byte row, byte column, KeyQueue q) {
//  //q.pressesToSend[row] = 
//  q.count = q.count + 1;
//}

void sendKeyPress(byte row, byte column) {
  Keyboard.press(KEYMAP[row][column]);
  return;
  char buf[4];
  sprintf (buf, "%c", KEYMAP[row][column]);
  Serial.println(buf);
}

void sendKeyRelease(byte row, byte column) {
  Keyboard.release(KEYMAP[row][column]);
}

void loop() {
  // this code needs to be rewritten with interrupts
  bool rows[7];
  int selectedColumn = 0;
  byte columnBitmap = 0xFF;
  bool switchOpen = true;
  byte last8Readings = 0xFF;
  bool switchState = false;
  bool stopScanningOff = true; // used to break out of scanning routine

  init();
  setup();

  disableAllColumns();
  selectedColumn = selectFirstColumn();
  // select column N only
  //for(int i=0 ; i<4 ; i++){
    //selectedColumn = selectNextColumn(selectedColumn, COLUMNCOUNT);
  //}
  
  while(stopScanningOff) {
    //delay(2);
    //Serial.println("scanning column ");
    //Serial.println(selectedColumn, 10);
    // read stopScanning button
    //stopScanningOff = digitalRead(SCANSTOP);
    //if(!stopScanningOff){ break; }
    
    columnBitmap = readRows(rows, 7, ROWSPINS);
    // check each switch of this column
    for(byte row=0; row<ROWCOUNT; row++) {
      //Serial.println("scanning row of column, row =");
      //Serial.println(row, 10);
      // Read switch on row row, column selectedColumn.
      switchOpen = (bool)digitalRead(ROWSPINS[row]);
      if(!switchOpen && row==1 && selectedColumn==0){ 
        Serial.println("stopping scan");
        stopScanningOff = false;
        break;
        Serial.print(row);
        Serial.print(",");
        Serial.print(selectedColumn);
        Serial.println(); 
      }
      last8Readings = matrixReadingsPush(row, selectedColumn, switchOpen);
      switchState = matrixStateGet(row, selectedColumn);
      if (last8Readings==0x0 && !switchState) { // stable press, non press to press.
        sendKeyPress(row, selectedColumn);
        matrixStateSet(row, selectedColumn, true);

      }else if (last8Readings==0xFF && switchState) { // stable depress, press to non press.
        sendKeyRelease(row, selectedColumn);
        matrixStateSet(row, selectedColumn, false);
      }
    }
    selectedColumn = selectNextColumn(selectedColumn, COLUMNCOUNT);
  
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
