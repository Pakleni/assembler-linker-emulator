#include <stdio.h>
#include <string>
#include <vector>
#include <map>

#define DEBUG true

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

    void Finish();
    void addDataB(uint8_t);
    void addDataW(uint16_t);
    void addNonRelativeValue(std::string label, int offset);
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
public:
    enum RelTypes
    {
        R_PC16,
        R_16
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

class Operand
{
public:
    virtual int calculate() = 0;
};

class SymbolOp : public Operand
{
    std::string symbol;
    // pc = rel na sekciju + locationCounter

    // vrednost             <- simbol.vr            imm
    // vrednost iz mem abs  <- mem[simbol.vr]       memdir
    // vrednost iz mem rel  <- mem[pc + simbol.vr]  
    // vrednost abs         <- vrednost
    // vrednost rel         <- pc + simbol.vr
};

class LiteralOp : public Operand
{
    int literal;
    //v
    //mem[v]
};

class RegOp : public Operand
{
    int reg;
    //REGDIR
    //MEMDIR
};

class RegLiteralOp : public Operand
{
    int reg;
    int literal;
    //MEMDIR
};

class RegSymbolOp : public Operand
{  
    int reg;
    std::string symbol;
    //MEMDIR
};
