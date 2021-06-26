SRCS = src/main.cpp src/linker.cpp src/reader.cpp ../assembler/src/printer.cpp ../assembler/src/structures.cpp

all:
	g++ $(SRCS) -o out/linker -g3

test:
	make all
	hexyl tests/interrupts.o
	readelf -a tests/interrupts.o
	./out/linker -place=kme@0xFEFE -hex tests/interrupts.o tests/main.o