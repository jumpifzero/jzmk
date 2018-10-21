/*
  This is a sample code to test the build process (makefile). Not a full keyboard firmwar.e

  Emulates a keyboard with 4 keys in a 2x2 matrix.
  Needs 5 pins of the arduino:
   2 column pins (configured as digital output)
   2 row pins as input (pullup)
   the "keyboard enable" switch. For safety while developing.
  The circuit:
  - LED attached from pin 13 to ground
  Check pictures in the repo.
  - Note: on most Arduinos there is already an LED on the board
    attached to pin 13.
  created 2018
  by Tiago Almeida @jumpifzero
*/

#include <Keyboard.h>

const int enableBtnPin = 2;     // the number of the pushbutton pin
const int ledPin =  13;         // the number of the LED pin
const int c0Pin = 8;
const int c1Pin = 9;
const int r0Pin = 4;
const int r1Pin = 7;

bool keyboardStarted = false;
unsigned int scanColumn = 0;
int pinForColumn[] = { 8, 9 };
int keyPress[2][2] = {{ false, false }, { false, false }};
int prevKeyPress[2][2] = {{ false, false }, { false, false }};
char character[2][2] = { {'a', 'b'}, {'c', 'd'} };
bool debugSerial = false;

void setup() {
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(enableBtnPin, INPUT);
  // column pins are outputs and set to high
  pinMode(c0Pin, OUTPUT);
  pinMode(c1Pin, OUTPUT);
  // row pins are input with pullups
  pinMode(r0Pin, INPUT_PULLUP);
  pinMode(r1Pin, INPUT_PULLUP);
  // columns are set high
  digitalWrite(c0Pin, HIGH);
  digitalWrite(c1Pin, HIGH);
  if (debugSerial) {
    Serial.begin(9600);
  }
}

void loop() {
  int r0;
  int r1;
  int keyboardEnabled = (digitalRead(enableBtnPin) == HIGH);

  delay(10);

  if (keyboardEnabled) {
    digitalWrite(ledPin, HIGH); // LED is on when keyboard enabled.

    if ( !keyboardStarted ) { 
      keyboardStarted = true;
      Keyboard.begin();
    }

    // copy the previous matrix state for this column;
    prevKeyPress[0][scanColumn] = keyPress[0][scanColumn];
    prevKeyPress[1][scanColumn] = keyPress[1][scanColumn];

    // clear the current matrix state for this column
    keyPress[0][scanColumn] = false;
    keyPress[1][scanColumn] = false;
    
    // we set the scanColumn to low. 
    // then we read the state of the rows. 
    // then we increment the column modulus 2.
    // we record the state of the keys in the keyPress.
    // when we get to the last column, for every key, we compare previous scan to current
    // to decide if we send a keypress or a keyrelease comand (or nothing)
    digitalWrite(pinForColumn[scanColumn], LOW); 
    r0 = digitalRead(r0Pin);
    r1 = digitalRead(r1Pin);
    digitalWrite(pinForColumn[scanColumn], HIGH); 
    if ( r0 == LOW ) { 
      keyPress[0][scanColumn] = true;
    }
    if ( r1 == LOW ) { 
      keyPress[1][scanColumn] = true;
    }
  
    // only when the scanColumn reaches the final, do we actually send anything
    if ( scanColumn == 1) { 
      // now we send a press or a release by comparing previous to current matrix states
      for(int r=0 ; r<2 ; r++){
        for(int c=0 ; c<2 ; c++){
          if (!prevKeyPress[r][c] && keyPress[r][c]){ 
            Keyboard.press(character[r][c]);  
            if (debugSerial){
              Serial.println("press");
              Serial.println(r);
              Serial.println(c);
              Serial.println(character[r][c]);  
            }
          }
          if(!keyPress[r][c] && prevKeyPress[r][c]) { 
            Keyboard.release(character[r][c]);  
            if (debugSerial){
              Serial.println("unpress");
              Serial.println(r);
              Serial.println(c);
              Serial.println(character[r][c]);
            }
          }
        }
      }  
    }

    scanColumn = (scanColumn + 1) % 2;
    
  } else { // when the keyboard enable switch is released we disable the keyboard lib.
    digitalWrite(ledPin, LOW);
     
    if ( keyboardStarted ) { 
      Keyboard.end();
      keyboardStarted = false;
    }
    digitalWrite(ledPin, LOW);   
  }

}
