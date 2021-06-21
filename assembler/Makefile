SRCS = src/main.cpp out/parser.cpp out/lexer.cpp src/assembler.cpp

lexer:
	flex src/lexer.l

parser:
	bison src/parser.y

all:
	$(MAKE) parser
	$(MAKE) lexer
	g++ $(SRCS) -o out/assembler

test:
	$(MAKE) clean
	$(MAKE) all
	./out/assembler -o out/test.o tests/test.s

clean:
	rm -f out/*