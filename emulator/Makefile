SRCS = src/main.cpp src/emulator.cpp

all:
	g++ $(SRCS) -o out/emulator

test:
	make all
	./out/emulator tests/test.hex