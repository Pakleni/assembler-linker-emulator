SRCS = src/main.cpp src/emulator.cpp

all:
	g++  -pthread -o out/emulator $(SRCS)

test:
	make all
	./out/emulator tests/test.hex