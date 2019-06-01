# 

# To build this:
# Make sure you install the 'arduino-mk' package in Ubuntu or Debian.
# If you install it in another place, adjust the include below to match
#
# More info: https://mjoldfield.com/atelier/2009/02/arduino-cli.html


BOARD_TAG    = pro5v328
ARDUINO_LIBS = Keyboard HID
#MONITOR_PORT = /dev/ttyACM0
include /usr/share/arduino/Arduino.mk


# --- sparkfun pro micro
#BOARD_TAG         = promicro16
#ALTERNATE_CORE    = promicro
#BOARDS_TXT        = $(HOME)/arduino/hardware/promicro/boards.txt
#BOOTLOADER_PARENT = $(HOME)/arduino/hardware/promicro/bootloaders
#BOOTLOADER_PATH   = caterina
#BOOTLOADER_FILE   = Caterina-promicro16.hex
#ISP_PROG          = usbasp
#AVRDUDE_OPTS      = -v
#include /usr/share/arduino/Arduino.mk


# --- atmega328p on breadboard
#BOARD_TAG    = atmega328bb
#ISP_PROG     = usbasp
#AVRDUDE_OPTS = -v
#BOARDS_TXT   = $(HOME)/arduino/hardware/breadboard/boards.txt
#include /usr/share/arduino/Arduino.mk

CC=g++
CFLAGS=-I.

clean:
	rm -rf ./*.o

test.o: test.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

test: test.o
	$(CC) -o test test.o
