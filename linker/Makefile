SRCS = src/main.cpp src/linker.cpp src/reader.cpp ../assembler/src/printer.cpp ../assembler/src/structures.cpp

all:
	g++ $(SRCS) -o out/linker -g3

test-hex:
	make all
	# hexyl tests/interrupts.o
	# readelf -a tests/interrupts.o
	./out/linker -place=ivt@0x0000 -o out/out.hex -hex tests/interrupts.o tests/main.o

test-unres:
	make all
	# hexyl tests/interrupts.o
	# readelf -a tests/interrupts.o
	./out/linker -place=ivt@0x0000 -o out/out.hex -hex tests/test.o tests/interrupts.o tests/main.o

test-link:
	make all
	# hexyl tests/interrupts.o
	# readelf -a tests/interrupts.o
	./out/linker -linkable tests/test.o tests/interrupts.o tests/test2.o tests/main.o