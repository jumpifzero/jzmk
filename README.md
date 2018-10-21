# jzmk
Firmware for an ATmega32u4 based mechanical keyboard

# About
This is a firmware to run on an ATmega32u4 board, like for example the Arduino Leonardo. 
The keyboard matrix has the rows as input and the columns as output which is the opposite way most keyboard matrices are setup. Also, the columns are connected through a cascade of 3 shift registers (e.g. 74HC595).

# Code
Check the `dev` branch.

# Compiling on the command line (Linux)
 1. Install the Arduino IDE
 1. Install the avr-gcc toolchain (e.g. for Ubuntu: `sudo apt-get install gcc-avr binutils-avr avr-libc`)
 1. Clone Arduino-Makefile. Example: `cd ~\bin && git clone https://github.com/sudar/Arduino-Makefile.git`
 1. Setup your environment variables (see below)
 1. run `make`
 
# Setup environment variables for compilation
Add the bellow to your `~\.bashrc` Adjusting paths as necessary. 

In my case, whereis avr-gcc reports `\usr\bin\avr-gcc` hence the \usr on `AVR_TOOLS_DIR` and I cloned the Arduino and Arduino-Makefile to `~\bin`.
`
export ARDUINO_DIR=/home/tiago/bin/arduino-1.8.7
export ARDMK_DIR=/home/tiago/bin/Arduino-Makefile
export AVR_TOOLS_DIR=/usr
`    

# TODO
Everything

