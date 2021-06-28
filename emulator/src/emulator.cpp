#include "../inc/emulator.hpp"
#include <iostream>
#include <termios.h>

#define WORD(m, l) ((((uint16_t)m[l + 1]) << 8) + m[l])
#define PC 7
#define SP 6
#define PSW 8

#define HALT 0b00000000
#define IRET 0b00100000
#define RET 0b01000000
#define CALL 0b00110000
#define JMP 0b01010000
#define JEQ 0b01010001
#define JNE 0b01010010
#define JGT 0b01010011
#define INT 0b00010000
#define NOT 0b10000000
#define XCHG 0b01100000
#define ADD 0b01110000
#define SUB 0b01110001
#define MUL 0b01110010
#define DIV 0b01110011
#define CMP 0b01110100
#define AND 0b10000001
#define OR 0b10000010
#define XOR 0b10000011
#define TEST 0b10000100
#define SHL 0b10010000
#define SHR 0b10010001
#define LDR 0b10100000
#define STR 0b10110000

#define IMM 0b0000
#define REGDIR 0b0001
#define REGDIRADD 0b0101
#define REGIND 0b0010
#define REGINDADD 0b0011
#define MEMDIR 0b0100

#define I (1 << 15)
#define Tl (1 << 14)
#define Tr (1 << 13)
#define N (1 << 3)
#define C (1 << 2)
#define O (1 << 1)
#define Z (1 << 0)

#define term_out 0xFF00
#define term_in 0xFF02

#define tim_cfg 0xFF10

using namespace std;

inline void set_mem16(uint8_t m[], uint16_t loc, uint16_t val)
{
    m[loc] = val & 0x00FF;
    m[loc + 1] = (val & 0xFF00) >> 8;
}

inline uint8_t get_rd(uint8_t RD)
{
    return (RD & 0x0F) >> 4;
}

inline uint8_t get_rs(uint8_t RD)
{
    return (RD & 0xF);
}

void Emulator::readHex(string in)
{
    FILE *file = fopen(in.c_str(), "r");

    int loc;
    int x;
    while (fscanf(file, "%X:", &loc) != EOF)
    {
        for (int i = 0; i < 8; i++)
        {
            fscanf(file, " %X", &x);
            memory[loc + i] = x;
        }
        fscanf(file, "\n");
    }
    fclose(file);
}

bool Emulator::parseInstruction()
{
    uint8_t ID = memory[regs[PC]++];

    switch (ID)
    {
    case (HALT):
        return false;
    case (IRET):
        iret();
        return true;
    case (RET):
        ret();
        return true;
    }

    uint8_t RD = memory[regs[PC]++];

    switch (ID)
    {
    case (INT):
        interrupt(RD);
        return true;
    case (XCHG):
        xchg(RD);
        return true;
    case (ADD):
        add(RD);
        return true;
    case (SUB):
        sub(RD);
        return true;
    case (MUL):
        mul(RD);
        return true;
    case (DIV):
        div(RD);
        return true;
    case (CMP):
        cmp(RD);
        return true;
    case (NOT):
        i_not(RD);
        return true;
    case (AND):
        i_and(RD);
        return true;
    case (OR):
        i_or(RD);
        return true;
    case (XOR):
        i_xor(RD);
        return true;
    case (TEST):
        i_test(RD);
        return true;
    case (SHL):
        shl(RD);
        return true;
    case (SHR):
        shr(RD);
        return true;
    }

    uint8_t AM = memory[regs[PC]++];

    uint8_t am = AM & 0x0F;
    uint8_t up = (AM & 0xF0) >> 4;


    uint16_t operand = 0;
    uint8_t rs = RD & 0x0F;
    if (am == REGDIR || am == REGIND)
    {

        if (ID == STR)
        {
            store(RD, am, up);
            return true;
        }

        if (am == REGIND)
        {
            switch (up)
            {
            case (0):
                operand = WORD(memory, regs[rs]);
                break;
            case (1):
                regs[rs] -= 2;
                operand = WORD(memory, regs[rs]);
                break;
            case (2):
                regs[rs] += 2;
                operand = WORD(memory, regs[rs]);
                break;
            case (3):
                operand = WORD(memory, regs[rs]);
                regs[rs] -= 2;
                break;
            case (4):
                operand = WORD(memory, regs[rs]);
                regs[rs] += 2;
                break;
            }
        }
        else if (am == REGDIR)
        {
            operand = regs[rs];
        }
    }
    else
    {
        uint8_t DH = memory[regs[PC]++];
        uint8_t DL = memory[regs[PC]++];

        uint16_t data = (DL << 8) + DH;

        if (ID == STR)
        {
            if (am = IMM)
            {
                error();
                return true;
            }
            store(RD, am, up, data);
            return true;
        }

        if (am == REGINDADD)
        {
            switch (up)
            {
            case (0):
                operand = WORD(memory, (regs[rs] + data));
                break;
            case (1):
                regs[rs] -= 2;
                operand = WORD(memory, (regs[rs] + data));
                break;
            case (2):
                regs[rs] += 2;
                operand = WORD(memory, (regs[rs] + data));
                break;
            case (3):
                operand = WORD(memory, (regs[rs] + data));
                regs[rs] -= 2;
                break;
            case (4):
                operand = WORD(memory, (regs[rs] + data));
                regs[rs] += 2;
                break;
            }
        }
        else if (am == REGDIRADD)
        {
            switch (up)
            {
            case (1):
                regs[rs] -= 2;
                break;
            case (2):
                regs[rs] += 2;
                break;
            }
            operand = regs[rs] + data;
            switch (up)
            {
            case (3):
                regs[rs] -= 2;
                break;
            case (4):
                regs[rs] += 2;
                break;
            }
        }
        else if (am == IMM)
        {
            operand = data;
        }
        else if (am == MEMDIR)
        {
            operand = WORD(memory, data);
        }
    }

    switch (ID)
    {
    case (CALL):
        call(operand);
        return true;
    case (JMP):
        jump(operand);
        return true;
    case (JEQ):
        jeq(operand);
        return true;
    case (JNE):
        jne(operand);
        return true;
    case (JGT):
        jgt(operand);
        return true;
    case (LDR):
        load(RD, operand);
        return true;
    }

    error();
    return true;
}

void Emulator::timerFunction()
{
    int time;
    uint16_t temp;

    while (isRunning)
    {
        memory_mutex.lock();
        temp = WORD(memory, tim_cfg);
        memory_mutex.unlock();

        switch (temp)
        {
        case (0):
            time = 500;
            break;
        case (1):
            time = 1000;
            break;
        case (2):
            time = 1500;
            break;
        case (3):
            time = 2000;
            break;
        case (4):
            time = 5000;
            break;
        case (5):
            time = 10000;
            break;
        case (6):
            time = 30000;
            break;
        case (7):
            time = 50000;
            break;
        }

        this_thread::sleep_for(chrono::milliseconds(time));
        timer = true;
    }
}

void Emulator::start(string in)
{
    isRunning = true;
    readHex(in);
    uint8_t *ep;
    regs[PC] = WORD(memory, 0);
    set_mem16(memory, tim_cfg, 0);

    thread t([]
             { Emulator::getInstance().timerFunction(); });
    t.detach();

    while (parseInstruction())
    {

        if (!(regs[PSW] & I))
        {
            if (!(regs[PSW] & Tr) && timer)
            {
                timer = false;
                timerInterrupt();
            }
            if (!(regs[PSW] & Tl) && terminal)
            {
                terminal = false;
                terminalInterrupt();
            }
        }

    }

    isRunning = false;
}

Emulator::~Emulator()
{
}

void Emulator::error()
{
    push(PC);
    push(PSW);
    regs[PC] = WORD(memory, 1 * 2);
}

void Emulator::timerInterrupt()
{
    push(PC);
    push(PSW);
    regs[PC] = WORD(memory, 2 * 2);
}

void Emulator::terminalInterrupt()
{
    push(PC);
    push(PSW);
    regs[PC] = WORD(memory, 3 * 2);
}

void Emulator::interrupt(uint8_t RD)
{
    uint8_t rd = get_rd(RD);
    push(PC);
    push(PSW);

    regs[PC] = WORD(memory, (regs[rd] % 8) * 2);
}

void Emulator::iret()
{
    pop(PSW);
    pop(PC);
}

void Emulator::call(uint16_t op)
{
    push(PC);
    regs[PC] = op;
}

void Emulator::ret()
{
    pop(PC);
}

void Emulator::jump(uint16_t op)
{
    regs[PC] = op;
}

void Emulator::jeq(uint16_t op)
{
    if (regs[PSW] & Z)
    {
        jump(op);
    }
}

void Emulator::jne(uint16_t op)
{
    if (!(regs[PSW] & Z))
    {
        jump(op);
    }
}

void Emulator::jgt(uint16_t op)
{
    if (!(regs[PSW] & Z) && (((regs[PSW] & C) && (regs[PSW] & C)) ||
                             (!(regs[PSW] & C) && !(regs[PSW] & C))))
    {
        jump(op);
    }
}

void Emulator::xchg(uint8_t RD)
{
    uint8_t rd = get_rd(RD);
    uint8_t rs = get_rs(RD);

    uint16_t temp = regs[rd];
    regs[rd] = regs[rs];
    regs[rs] = temp;
}

void Emulator::add(uint8_t RD)
{
    regs[get_rd(RD)] += regs[get_rs(RD)];
}

void Emulator::sub(uint8_t RD)
{
    regs[get_rd(RD)] -= regs[get_rs(RD)];
}

void Emulator::mul(uint8_t RD)
{
    regs[get_rd(RD)] *= regs[get_rs(RD)];
}

void Emulator::div(uint8_t RD)
{
    regs[get_rd(RD)] /= regs[get_rs(RD)];
}

void Emulator::cmp(uint8_t RD)
{
    int16_t temp = regs[get_rd(RD)] - regs[get_rs(RD)];

    if (temp == 0)
    {
        regs[PSW] |= Z;
    }
    else
    {
        regs[PSW] &= ~Z;
    }

    if (temp < 0)
    {
        regs[PSW] |= N;
    }
    else
    {
        regs[PSW] &= ~N;
    }

    if (regs[get_rd(RD)] < regs[get_rs(RD)])
    {
        regs[PSW] |= C;
    }
    else
    {
        regs[PSW] &= ~C;
    }

    if (((int16_t)regs[get_rd(RD)] < 0 &&
         (int16_t)regs[get_rs(RD)] > 0 &&
         temp > 0) ||
        ((int16_t)regs[get_rd(RD)] > 0 &&
         (int16_t)regs[get_rs(RD)] < 0 &&
         temp < 0))
    {
        regs[PSW] |= O;
    }
    else
    {
        regs[PSW] &= ~O;
    }
}

void Emulator::i_not(uint8_t RD)
{
    regs[get_rd(RD)] = ~regs[get_rd(RD)];
}

void Emulator::i_and(uint8_t RD)
{
    regs[get_rd(RD)] &= regs[get_rs(RD)];
}

void Emulator::i_or(uint8_t RD)
{
    regs[get_rd(RD)] |= regs[get_rs(RD)];
}

void Emulator::i_xor(uint8_t RD)
{
    regs[get_rd(RD)] ^= regs[get_rs(RD)];
}

void Emulator::i_test(uint8_t RD)
{
    int16_t temp = regs[get_rd(RD)] & regs[get_rs(RD)];

    if (temp == 0)
    {
        regs[PSW] |= Z;
        regs[PSW] &= ~N;
    }
    else
    {
        regs[PSW] &= ~Z;
        if (temp < 0)
        {
            regs[PSW] |= N;
        }
        else
        {
            regs[PSW] &= ~N;
        }
    }
}

void Emulator::shl(uint8_t RD)
{
    regs[get_rd(RD)] <<= regs[get_rs(RD)];
}

void Emulator::shr(uint8_t RD)
{
    regs[get_rd(RD)] >>= regs[get_rs(RD)];
}

void Emulator::load(uint8_t RD, uint16_t operand)
{
    regs[get_rd(RD)] = operand;
}

void Emulator::store(uint8_t RD, uint8_t am, uint8_t up)
{
    uint8_t rd = get_rd(RD);
    uint8_t rs = get_rs(RD);

    if (am == REGDIR)
    {
        regs[rs] = regs[rd];
    }
    else if (am == REGIND)
    {
        switch (up)
        {
        case (1):
            regs[rs] -= 2;
            break;
        case (2):
            regs[rs] += 2;
            break;
        }

        memory_mutex.lock();
        set_mem16(memory, regs[rs], regs[rd]);
        memory_mutex.unlock();

        switch (up)
        {
        case (3):
            regs[rs] -= 2;
            break;
        case (4):
            regs[rs] += 2;
            break;
        }
    }
}

void Emulator::store(uint8_t RD, uint8_t am, uint8_t up, uint16_t data)
{
    uint8_t rd = get_rd(RD);
    uint8_t rs = get_rs(RD);

    if (am == REGDIRADD)
    {
        error();
    }
    else if (am == REGINDADD)
    {
        switch (up)
        {
        case (1):
            regs[rs] -= 2;
            break;
        case (2):
            regs[rs] += 2;
            break;
        }
        
        memory_mutex.lock();
        set_mem16(memory, regs[rs] + data, regs[rd]);
        memory_mutex.unlock();

        switch (up)
        {
        case (3):
            regs[rs] -= 2;
            break;
        case (4):
            regs[rs] += 2;
            break;
        }
    }
    else if (am == MEMDIR)
    {
        memory_mutex.lock();
        set_mem16(memory, data, regs[rd]);
        memory_mutex.unlock();
    }
}

void Emulator::push(uint8_t reg)
{
    regs[SP] -= 2;
    memory_mutex.lock();
    set_mem16(memory, regs[SP], regs[reg]);
    memory_mutex.unlock();
}

void Emulator::pop(uint8_t reg)
{
    regs[reg] = WORD(memory, regs[SP]);
    regs[SP] += 2;
}