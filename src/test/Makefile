.PHONY: jzmk copy-ino

INC=./

clean:
	rm -rf ./*.o

copy-ino:
	cp ../../jzmk.ino ./jzmk.cpp

copy-headers:
	cp ../../*.h ./

copy-cpp:
	cp ../../*.cpp ./

jzmk: ../../jzmk.ino
	g++ -g -I$(INC) -c jzmk.cpp -o jzmk.o

mock-arduino.o:
	g++ -g -c mock-arduino.cpp

test: test.o mock-arduino.o copy-ino copy-headers copy-cpp jzmk
	g++ -g -I$(INC) -o test mock-arduino.o jzmk.o test.o
