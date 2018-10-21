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
#define COLSRDATA 0  // push the bit to this pin
#define COLSRCLOCK 0 // pulse this pin L-H-L to shift in the data
#define COLSRLATCH 0 // pulse this pin L-H-L to move the data shifted in to the output.


/**
 * Sets all columns connected to the shift registers to be High
 **/
void disableAllColumns() {
  digitalWrite(COLSRLATCH, LOW);
  for(int i=0 ; i<MAXSR*8 ; i++) {
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
  shiftOut(COLSRDATA, COLSRCLOCK, LSBFIRST, 0); // shift out a Low
  digitalWrite(COLSRLATCH, HIGH);
  digitalWrite(COLSRLATCH, LOW);
  return 0;
}

byte selectNextColumn(byte currentColumn) {
  digitalWrite(COLSRLATCH, LOW);
  shiftOut(COLSRDATA, COLSRCLOCK, LSBFIRST, 1); // shift out a High.
  digitalWrite(COLSRLATCH, HIGH);
  digitalWrite(COLSRLATCH, LOW);
  return currentColumn + 1;
}
