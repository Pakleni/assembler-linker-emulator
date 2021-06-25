#include <stdio.h>
#include <string>

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