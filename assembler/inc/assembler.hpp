#include <stdio.h>
#include "operand.hpp"
#include "structures.hpp"
#include <string>
#include <vector>
#include <map>

#define DEBUG false

struct IdentList
{
    IdentList(std::string _val) : val(_val){};
    IdentList(std::string _val, IdentList *_next) : val(_val), next(_next){};
    ~IdentList() { delete next; }

    std::string val;
    IdentList *next = nullptr;
};

struct IdentList;

class Assembler
{
private:
    Assembler(){};

public:
    ~Assembler();
    Assembler(Assembler const &) = delete;
    void operator=(Assembler const &) = delete;
    static Assembler &getInstance()
    {
        static Assembler instance;
        return instance;
    }

    int locationCounter = 0;
    int currentSection = -1;
    bool firstPass = true;

    std::vector<SymTabEntry *> SymTab;
    std::map<std::string, SymTabEntry *> SymMap;
    std::vector<Section *> Sections;

    //change to second pass
    void secondPass();
    //add symbol to symtab
    SymTabEntry *addSymbol(std::string label);
    //functions to be called while parsing
    void parseGlobal(IdentList *list);
    void parseExtern(IdentList *list);
    void parseSection(std::string name);
    void parseWord(IdentList *list);
    void parseWord(uint16_t literal);
    void parseSkip(int literal);
    void parseEqu(std::string ident, int literal);
    void parseEnd();
    void parseLabel(std::string ident);

    void addDataB(uint8_t);
    void addDataW(uint16_t);
    void addData3B(uint32_t);
    void addNonRelativeValue(std::string label);
    void addRelativeValue(std::string label);

    //standalone instructions
    void push(int);
    void pop(int);

    void noaddr(uint8_t);
    void tworeg(uint8_t instr, uint8_t rd, uint8_t rs);
    void jmp(uint8_t instr, Operand *op);
    void regop(uint8_t instr, uint8_t rd, Operand *op);

    bool humanReadable = false;
    void setHumanReadable(bool b) { humanReadable = b; }

    void Finish();
};

