#include <vector>
#include "structures.hpp"
#include "../inc/elf_structures.hpp"
#include <string>

class ELFFile
{
public:
    std::vector<Section *> sections;
    std::vector<SymTabEntry *> symtab;
    ~ELFFile();
    void addSymbol(std::string label, ELFSTEntry * entry);
    void addSection(std::string label);
    void addRel(ELFRelEntry * entry, ELFSHeader * header);


};

class Reader
{
public:
    std::string filename;

    Reader(std::string _filename) : filename(_filename) {}
    ELFFile *read();
};