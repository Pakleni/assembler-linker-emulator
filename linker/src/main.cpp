#include "../inc/linker.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#define DEFAULT_OUTPUT "out/out"

bool checkPlace(std::string s) {

    auto i = s.find("=");

    if (i != std::string::npos) {
        return (s.substr(0, i).compare("-place") == 0);
    }
    return false;
}

int main(int argc, char* argv[])
{
    if (argc < 2) return EXIT_FAILURE;

    const char * out = DEFAULT_OUTPUT;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            out = argv[++i];
        }
        else if(strcmp(argv[i], "-linkable") == 0) {
            Linker::getInstance().setMode(Linker::linkable);
        }
        else if(strcmp(argv[i], "-hex") == 0) {
            Linker::getInstance().setMode(Linker::hex);
        }
        else if(checkPlace(argv[i])){
            Linker::getInstance().parsePlace(argv[i]);
        }
        else {
            Linker::getInstance().addSource(argv[i]);
        }
    }
    
    Linker::getInstance().start(out);

    return EXIT_SUCCESS;
}