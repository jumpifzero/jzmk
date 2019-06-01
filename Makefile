# 

# To build this:
# Make sure you install the 'arduino-mk' package in Ubuntu or Debian.
# If you install it in another place, adjust the include below to match
#
# More info: https://mjoldfield.com/atelier/2009/02/arduino-cli.html
#
# Install arduino-makefile: sudo apt-get install arduino-mk


MCU = atmega32u4
BOARD_TAG    = leonardo
ARDUINO_LIBS = Keyboard HID EEPROM
ARDUINO_DIR  = /home/tiago/bin/arduino-1.8.7
ARDMK_DIR	   = /usr/share/arduino/
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
is:open#BOARD_TAG    = atmega328bb
#ISP_PROG     = usbasp
#AVRDUDE_OPTS = -v
#BOARDS_TXT   = $(HOME)/arduino/hardware/breadboard/boards.txt
#include /usr/share/arduino/Arduino.mk

CFLAGS=-I.
OTHER_OBJS=MacroPersistance.o

#clean:
#	rm -rf ./*.o

#test.o: test.cpp
#	$(CC) -c -o $@ $< $(CFLAGS)

#test: src/test/test.o
#	gcc -o src/test/test src/test/test.o
