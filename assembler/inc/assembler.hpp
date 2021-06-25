#include <stdio.h>
#include "operand.hpp"
#include "printer.hpp"
#include <string>
#include <vector>
#include <map>

#define DEBUG false

#define ABSOLUTE_SECTION -1
#define EXTERNAL_SECTION -2

class SymTabEntry;
class RelEntry;
class Section;
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
    void addNonRelativeValue(std::string label, int offset);
    void addRelativeValue(std::string label, int offset);

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

class SymTabEntry
{
private:
    static int idc;

public:
    int id = idc++;

    std::string label;
    //dobija id trenutnog section-a
    int section = Assembler::getInstance().currentSection;
    //dobija trenutnu vrednost location counter-a
    int offset = Assembler::getInstance().locationCounter;
    //local u prvom prolazu
    bool isLocal = true;
    int string_table_i = 0;
    bool isSection = false;

    SymTabEntry(std::string _label) : label(_label) {}
};

class RelEntry
{
public:
    enum RelTypes
    {
        R_16,
        R_PC16
    };
    int offset = Assembler::getInstance().locationCounter;
    RelTypes relType;

    int entry;

    RelEntry(int _offset, RelTypes _relType, int _entry) : relType(_relType),
                                                           entry(_entry)
    {
        offset += _offset;
    }
};

class Section
{
public:
    std::string name;
    std::vector<RelEntry *> rel;
    std::vector<uint8_t> data;

    int string_table_i = 0;
    int string_table_reli = 0;
    int offset = -1;
    int rel_offset = -1;


    //symbol id
    int id;

    Section(std::string _name, int _id) : name(_name), id(_id) {}
    ~Section()
    {
        while (!rel.empty())
        {
            delete rel.back();
            rel.pop_back();
        }
    }
};

struct IdentList
{
    IdentList(std::string _val) : val(_val){};
    IdentList(std::string _val, IdentList *_next) : val(_val), next(_next){};
    ~IdentList() { delete next; }

    std::string val;
    IdentList *next = nullptr;
};