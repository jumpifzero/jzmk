/*

*/

#include <Keyboard.h>

// Rows and Coumns pin definitions
#define MAXSR 3 // Number of cascaded shift registers for the columns
#define COLUMNCOUNT 17 // Number of columns connected to the shift registers
#define ROWCOUNT 7 // Number of rows connected to the arduino. The code assumes maximum is 8 ! 

// Rows are directly connected
#define ROW0PIN 0
#define ROW1PIN 0
#define ROW2PIN 0
#define ROW3PIN 0
#define ROW4PIN 0
#define ROW5PIN 0
#define ROW6PIN 0

// Columns are connected through shift registers
#define COLSRDATA 11  // push the bit to this pin
#define COLSRCLOCK 12 // pulse this pin L-H-L to shift in the data
#define COLSRLATCH 8  // pulse this pin L-H-L to move the data shifted in to the output.

// global constants
// ----------------
const byte[] ROWSPINS[7] = {ROW0PIN,ROW1PIN,ROW2PIN,ROW3PIN,ROW4PIN,ROW5PIN,ROW6PIN}; // The pins used to connect to each row.

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
  delay(10);
  digitalWrite(COLSRLATCH, LOW);
}

/**
 * Selects the first column by setting it low.
 * Returns 0. The selected column.
 **/
byte selectFirstColumn() {
  digitalWrite(COLSRLATCH, LOW);
  digitalWrite(COLSRDATA,0);

  digitalWrite(COLSRCLOCK,HIGH);
  delay(10);
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
  digitalWrite(COLSRLATCH, LOW);
  digitalWrite(COLSRDATA, HIGH);
  digitalWrite(COLSRCLOCK, HIGH);
  delay(10);
  digitalWrite(COLSRCLOCK, LOW);
  digitalWrite(COLSRLATCH, HIGH);
  digitalWrisudo apt-get upgradete(COLSRLATCH, LOW);
  digitalWrite(COLSRDATA, LOW);
  return (currentColumn + 1) % numColumns;
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
  pinMode(COLSRDATA, OUTPUT);
  pinMode(COLSRCLOCK, OUTPUT);
  pinMode(COLSRLATCH, OUTPUT);
  // set all outputs in the shift register as high
  disableAllColumns();
}

typedef struct KeyQueue_s{
  byte count = 0;
  byte pressesToSend[8];
} KeyQueue;

void keyQueueInsert(byte row, byte column, KeyQueue q) {
  //q.pressesToSend[row] = 
  q.count = q.count + 1;

}

int main() {
  // this code needs to be rewritten with interrupts
  bool rows[7];
  int selectedColumn = 0;
  byte columnsBitmap = 0xFF;
  bool switchOpen = true;
  byte last8Readings = 0xFF;
  bool switchState = false;
  struct KeyQueue pressesToSend;

  init();
  setup();

  selectedColumn = selectFirstColumn();
  for(;;) {
    columnsBitmap = readRows(rows, 7, ROWSPINS);
    for(byte row=0; row<ROWCOUNT; row++) {
      // Read switch on row row, column selectedColumn.
      switchOpen = (bool)digitalRead(ROWSPINS[row]);
      last8Readings = matrixReadingsPush(row, selectedColumn, switchOpen);
      switchState = matrixStateGet(row, selectedColumn);
      if (last8Readings==0x0 && !switchState) { // stable press, non press to press.

        matrixStateSet(row, selectedColumn, true);

      }else if (last8Readings==0xFF && switchState) { // stable depress, press to non press.
        matrixStateSet(row, selectedColumn, true);
      }
    }

  }
}
