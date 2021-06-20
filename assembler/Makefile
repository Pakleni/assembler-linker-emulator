SRCS = out/parser.c out/lexer.c

lexer:
	flex src/lexer.l

parser:
	bison src/parser.y

all:
	make parser
	make lexer
	gcc $(SRCS) -o out/parser

test:
	make clean
	make all
	cat tests/test.s | ./out/parser

clean:
	rm -f out/*