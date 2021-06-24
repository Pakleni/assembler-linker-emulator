#include <stdio.h>
#include <string>
#include <vector>
#include <map>

#define DEBUG true

class SymTabEntry;
class RelEntry;
class Section;
struct IdentList;
class Operand;

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
    virtual uint16_t calculate() = 0;
    virtual int getSize() = 0;
};

class SymOp : public Operand
{
public:
    enum Mode
    {
        // vrednost             <- simbol.vr            imm
        DATA_DOLLAR,
        // vrednost iz mem abs  <- mem[simbol.vr]       memdir
        DATA_NULL,
        // vrednost iz mem rel  <- mem[pc + simbol.vr]
        DATA_PERCENT,
        // vrednost abs         <- vrednost
        JMP_NULL,
        // vrednost rel         <- pc + simbol.vr
        JMP_PERCENT,
        // vrednost iz mem abs  <- mem[simbol.vr]       memdir
        JMP_TIMES,
    };
    std::string symbol;
    Mode mode;
    SymOp(std::string _sym, Mode _m) : symbol(_sym), mode(_m){};

    uint16_t calculate();
    int getSize() { return 5; }
};

class LitOp : public Operand
{
public:
    enum Mode
    {
        DATA_DOLLAR,
        DATA_NULL,
        JMP_NULL,
        JMP_TIMES,
    };

    int literal;
    Mode mode;
    LitOp(int _lit, Mode _m) : literal(_lit), mode(_m){};

    uint16_t calculate();
    int getSize() { return 5; }
};

class RegOp : public Operand
{
    int reg;
    //REGDIR
    //MEMDIR
};

class RegLitOp : public Operand
{
    int reg;
    int literal;
    //MEMDIR
};

class RegSymOp : public Operand
{
    int reg;
    std::string symbol;
    //MEMDIR
};
