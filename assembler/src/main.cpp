#include <stdio.h>
#include "../out/parser.hpp"
#include "../out/lexer.hpp"
#include "../inc/assembler.hpp"


#define DEFAULT_OUTPUT "out/obj.o"

int main(int argc, char* argv[])
{

    if (argc < 2) return EXIT_FAILURE;

    freopen(argv[argc-1], "r", stdin);

    const char * out = DEFAULT_OUTPUT;

    for (int i = 1; i < argc-1; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            out = argv[++i];
        }
    }
    
    Assembler::getInstance().setOut(out);

    if (yyparse()) {
        return EXIT_FAILURE;
    };

    fclose(stdin);
    freopen(argv[argc-1], "r", stdin);

    if (yyparse()) {
        return EXIT_FAILURE;
    };

    return EXIT_SUCCESS;
}