.PHONY: jzmk copy-ino

INC=./mock/

copy-ino:
	cp ../../jzmk.ino ./jzmk.cpp

copy-headers:
	cp ../../*.h ./

copy-cpp:
	cp ../../*.cpp ./

jzmk: ../../jzmk.ino
	g++ -I$(INC) -c jzmk.cpp -o jzmk.o

mock-arduino.o:
	g++ -c mock-arduino.cpp

test: test.o mock-arduino.o copy-ino copy-headers copy-cpp jzmk
	g++ -I$(INC) -o test test.o mock-arduino.o
