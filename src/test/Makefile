.PHONY: jzmk copy-ino

INC=./

copy-ino:
	cp ../../jzmk.ino ./jzmk.cpp

copy-headers:
	cp ../../*.h ./

copy-cpp:
	cp ../../*.cpp ./

jzmk: ../../jzmk.ino
	g++ -I$(INC) -c jzmk.cpp -o jzmk.o

test: test.o copy-ino copy-headers copy-cpp jzmk
	g++ -I$(INC) -o test test.o
