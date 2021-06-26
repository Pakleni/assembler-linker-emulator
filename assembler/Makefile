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
	./out/assembler -o out/test.o tests/test.s
	hexyl out/test.o
	readelf -a out/test.o

test-human:
	$(MAKE) clean
	$(MAKE) all
	./out/assembler -h -o out/test-human.o tests/test.s

clean:
	rm -f out/*