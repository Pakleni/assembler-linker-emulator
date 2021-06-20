SRCS = out/parser.c out/lexer.c

lexer:
	flex src/lexer.l

parser:
	bison src/parser.y

all:
	$(MAKE) parser
	$(MAKE) lexer
	gcc $(SRCS) -o out/parser

test:
	$(MAKE) clean
	$(MAKE) all
	cat tests/test.s | ./out/parser

clean:
	rm -f out/*