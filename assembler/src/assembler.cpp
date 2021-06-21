#include <stdio.h>
#include "../out/parser.hpp"
#include "../out/lexer.hpp"


int main(int argc, char* argv[])
{

    if (argc < 2) return EXIT_FAILURE;

    freopen(argv[argc-1], "r", stdin);

    if (argc > 2 && strcmp(argv[1], "-o") == 0) freopen(argv[2], "w+", stdout);

	yyparse();

    return EXIT_SUCCESS;
}