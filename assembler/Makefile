SRCS = src/main.cpp out/parser.cpp out/lexer.cpp src/assembler.cpp src/operand.cpp src/printer.cpp src/structures.cpp

all:
	$(MAKE) parser
	$(MAKE) lexer
	g++ $(SRCS) -o out/assembler

lexer:
	flex src/lexer.l

parser:
	bison src/parser.y

test:
	$(MAKE) clean
	$(MAKE) all
	./out/assembler -o out/interrupts.o tests/interrupts.s
	./out/assembler -o out/main.o tests/main.s
	# hexyl out/test.o
	# readelf -a out/test.o

clean:
	rm -f out/*