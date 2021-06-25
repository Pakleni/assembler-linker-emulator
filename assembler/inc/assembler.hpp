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
    void HumanReadableOutput();
    void Output();
    void Finish();
    void BinaryOutput();
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
    void Write();
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
public:
    int reg;
    enum Mode
    {
        NONE,
        BRACKET,
    };

    Mode mode;
    RegOp(int _reg, Mode _m) : reg(_reg), mode(_m){};

    uint16_t calculate();
    int getSize() { return 3; }
};

class RegLitOp : public Operand
{
public:
    int reg;
    int literal;

    RegLitOp(int _reg, int _lit) : reg(_reg), literal(_lit){};

    uint16_t calculate();
    int getSize() { return 5; }
};

class RegSymOp : public Operand
{
public:
    int reg;
    std::string symbol;

    RegSymOp(int _reg, std::string _sym) : reg(_reg), symbol(_sym){};

    uint16_t calculate();
    int getSize() { return 5; }
};
