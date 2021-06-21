#include <stdio.h>
#include <string>
#include <vector>

class SymTabEntry;
class RelEntry;
class DataEntry;
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
    std::vector<RelEntry *> Rel;
    std::vector<DataEntry *> Data;
    std::vector<std::string> Sections;

    //change to second pass
    void secondPass();
    //add symbol to symtab
    SymTabEntry *addSymbol(std::string label);
    //functions to be called while parsing
    void parseGlobal(IdentList *list);
    void parseExtern(IdentList *list);
    void parseSection(std::string name);
    void parseWord(IdentList *list);
    void parseWord(int literal);
    void parseSkip(int literal);
    void parseEqu(std::string ident, int literal);
    void parseEnd();
    void parseLabel(std::string ident);
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

    SymTabEntry(std::string _label) : label(_label) {}
};

class RelEntry
{
    enum RelTypes
    {
        PCRel,
        Absolute
    };
    int section;
    int offset;
    int addend;
    RelTypes relType;
};

class DataEntry
{
public:
    int section;
    int offset;
    int data;
};

struct IdentList
{
    IdentList(std::string _val) : val(_val){};
    IdentList(std::string _val, IdentList *_next) : val(_val), next(_next){};
    ~IdentList() { delete next; }

    std::string val;
    IdentList *next = nullptr;
};