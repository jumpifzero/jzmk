/*

*/

#include <Keyboard.h>

// Rows and Coumns pin definitions
#define MAXSR 3 // Number of cascaded shift registers for the columns

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
const byte[] ROWSPINS[7] = {0,0,0,0,0,0,0}; // The pins used to connect to each row.


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
  digitalWrite(COLSRLATCH, LOW);
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

int main() {
  bool rows[7];
  int selectedColumn = 0;
  byte rowsBitMap = 0xFF;

  init();
  setup();
  // this code needs to be rewritten with interrupts
  selectedColumn = selectFirstColumn();
  rowsBitMap = readRows(rows, 7, ROWSPINS);
  if( rowsBitMap != 0xFF ) { // one of the switches seems to be on.
    
  } else {
    
  }
  for(;;) {
    
  }
}
