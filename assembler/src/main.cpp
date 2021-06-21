#include <stdio.h>
#include "../out/parser.hpp"
#include "../out/lexer.hpp"
#include "../inc/assembler.hpp"


const char* DEFAULT_OUTPUT = "out/obj.o";

int main(int argc, char* argv[])
{

    if (argc < 2) return EXIT_FAILURE;

    freopen(argv[argc-1], "r", stdin);

    if (argc > 2 && strcmp(argv[1], "-o") == 0) freopen(argv[2], "w+", stdout);
    else freopen(DEFAULT_OUTPUT, "w+", stdout);

    if (yyparse()) {
        return EXIT_FAILURE;
    };

    secondPass();

    if (yyparse()) {
        return EXIT_FAILURE;
    };

    return EXIT_SUCCESS;
}