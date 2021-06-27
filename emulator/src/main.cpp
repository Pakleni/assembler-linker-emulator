#include "../inc/emulator.hpp"
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if (argc < 2) return EXIT_FAILURE;

    char * input = argv[argc - 1];
    
    Emulator::getInstance().start(input);
    
    return EXIT_SUCCESS;
}