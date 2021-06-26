SRCS = src/main.cpp src/linker.cpp src/structures.cpp src/reader.cpp src/printer.cpp

all:
	g++ $(SRCS) -o out/linker

test:
	make all
	# hexyl tests/test.o
	# readelf -a tests/test.o
	./out/linker -place=kme@0xFEFE -hex tests/test.o