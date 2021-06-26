SRCS = src/main.cpp src/linker.cpp src/reader.cpp ../assembler/src/printer.cpp ../assembler/src/structures.cpp

all:
	g++ $(SRCS) -o out/linker -g3

test:
	make all
	# hexyl tests/test.o
	# readelf -a tests/test.o
	./out/linker -place=kme@0xFEFE -hex tests/test.o