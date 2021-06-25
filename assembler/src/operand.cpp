#include "../inc/assembler.hpp"

uint16_t SymOp::calculate() {

    uint16_t h = 0;
    switch(mode) {
        //rd = x, up = 0 uvek
        case(Mode::DATA_DOLLAR):
        case(Mode::JMP_NULL):
        //rs = x, imm
        h = 0x0000;
        break;
        case(Mode::DATA_NULL):
        case(Mode::JMP_TIMES):
        //rs = x, memdir
        h = 0x0004;
        break;
        case(Mode::DATA_PERCENT):
        //rs = PC, regindpom
        h = 0x0703;
        break;
        case(Mode::JMP_PERCENT):
        //rs = PC, regdiradd
        h = 0x0705;
        break;
    }

    if (mode == Mode::JMP_PERCENT || mode == Mode::DATA_PERCENT) {
        Assembler::getInstance().addRelativeValue(symbol, 0);
    }
    else {
        Assembler::getInstance().addNonRelativeValue(symbol, 0);
    }

    //SymOp instrukcije su uvek 5 bajtova
    Assembler::getInstance().locationCounter+=2;

    return h;
}

uint16_t LitOp::calculate() {

    uint16_t h = 0;
    switch(mode) {
        //rd = x, up = 0 uvek
        case(Mode::DATA_DOLLAR):
        case(Mode::JMP_NULL):
        //rs = x, imm
        h = 0x0000;
        break;
        case(Mode::DATA_NULL):
        case(Mode::JMP_TIMES):
        //rs = x, memdir
        h = 0x0004;
        break;
    }


    Assembler::getInstance().addDataW(literal);

    //LitOp instrukcije su uvek 5 bajtova
    Assembler::getInstance().locationCounter+=2;

    return h;
}

uint16_t RegOp::calculate() {

    uint16_t h = 0;
    switch(mode) {
        //rd = x, up = 0 uvek
        case(Mode::NONE):
        //rs = reg, regdir
        h = 0x0001 +(reg << 8);
        break;
        case(Mode::BRACKET):
        //rs = reg, regind
        h = 0x0002 +(reg << 8);
        break;
    }

    return h;
}

uint16_t RegLitOp::calculate() {

    //rs = reg, regindpom
    uint16_t h = 0x0003 +(reg << 8);;

    Assembler::getInstance().addDataW(literal);

    //RegLitOp instrukcije su uvek 5 bajtova
    Assembler::getInstance().locationCounter+=2;

    return h;
}

uint16_t RegSymOp::calculate() {

    //rs = reg, regindpom
    uint16_t h = 0x0003 +(reg << 8);

    Assembler::getInstance().addNonRelativeValue(symbol, 0);

    //RegSymOp instrukcije su uvek 5 bajtova
    Assembler::getInstance().locationCounter+=2;

    return h;
}