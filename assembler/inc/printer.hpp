#include <stdio.h>
#include <string>
#include <vector>
#include "structures.hpp"

class Printer
{
public:
    ~Printer(){};
    std::vector<Section *> sections;
    std::vector<SymTabEntry *> symtab;

    Printer(std::vector<Section *> _sec,
            std::vector<SymTabEntry *> _st,
            FILE *_file) : sections(_sec), symtab(_st), file(_file){};

    void Print();
    void HumanPrint();

    FILE *file;

private:
    int offset;
    int getSHStrTableSize();
    int getStrTableSize();
    int getSHOffset();
    void printELFHeader();
    int printSHStrTable();
    int printStrTable();
    int printSymTable();
    void printDataSections();
    void printRelSections();
    void printSHTable(int shstrttabsize, int symtabsize, int strtabsize);
    void writeB(uint8_t b)
    {
        fprintf(stdout, "%c", b);
    }
};