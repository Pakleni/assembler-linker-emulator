#include "../inc/assembler.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

const char* RelTypesString[] = {"R_16", "R_PC16"};

const int ABSOLUTE_SECTION = -1;
const int EXTERNAL_SECTION = -2;

void log(string c)
{
    fprintf(stderr, "%s", (c + "\n").c_str());
}

void debug(string c)
{
    if (DEBUG)
        log(c);
}

void Assembler::addDataB(uint8_t byte) {
    Sections[currentSection]->data.push_back(byte);
}

void Assembler::addDataW(uint16_t word) {
        uint8_t h = (word & 0xFF00 ) >> 8;
        uint8_t l =  word & 0x00FF;

        addDataB(l);
        addDataB(h);
}

void Assembler::addData3B(uint32_t word) {
        uint8_t l1 = (word & 0x00FF0000 ) >> 16;
        uint8_t l2 = (word & 0x0000FF00 ) >> 8;
        uint8_t l3 =  word & 0x000000FF;

        addDataB(l3);
        addDataB(l2);
        addDataB(l1);
}

void Assembler::addNonRelativeValue(string label, int offset) {
    auto s = SymMap.find(label);
    SymTabEntry *sym;
    int id;

    if (s == SymMap.end()) {
        sym = addSymbol(label);
        sym->isLocal = false;
        sym->offset = 0;
        sym->section = EXTERNAL_SECTION;
    }
    else {
        sym = s->second;

    }

    if (sym->section == ABSOLUTE_SECTION) {
        addDataW(sym->offset);
    }
    else {
        if (!sym->isLocal) {
            addDataW(0);
            id = sym->id;
        }
        else {
            addDataW(sym->offset); //addend
            id = Sections[sym->section]->id;
        }

        Sections[currentSection]->rel.push_back(new RelEntry(offset, RelEntry::R_16, id));
    }
}

void Assembler::addRelativeValue(string label, int offset) {
    auto s = SymMap.find(label);
    SymTabEntry *sym;
    int id;

    if (s == SymMap.end()) {
        sym = addSymbol(label);
        sym->isLocal = false;
        sym->offset = 0;
        sym->section = EXTERNAL_SECTION;
    }
    else {
        sym = s->second;
    }

    if (sym->section == ABSOLUTE_SECTION) {
        //nema smisla
        log ("Syntax error: You are trying to relative jump to an absolute position");
        exit(EXIT_FAILURE);
    }
    else if (sym->section == currentSection) {
        addDataW(sym->offset - locationCounter - 5); //relative jump
    }
    else {
        
        if (!sym->isLocal) {
            addDataW( -5);
            id = sym->id;
        }
        else {
            addDataW(sym->offset - 5); //addend
            id = Sections[sym->section]->id;
        }

        Sections[currentSection]->rel.push_back(new RelEntry(offset, RelEntry::R_PC16, id));
    }
}

int SymTabEntry::idc = 0;

void Assembler::secondPass()
{
    locationCounter = 0;
    firstPass = false;
    currentSection = -1;
}

SymTabEntry *Assembler::addSymbol(string label)
{
    if (SymMap.count(label))
    {
        log("Symbol with name: " + label + " already exists!");
        exit(EXIT_FAILURE);
    }

    SymTabEntry *c = new SymTabEntry(label);

    SymMap.insert({label, c});
    SymTab.push_back(c);

    return c;
}

void Assembler::parseGlobal(IdentList *list)
{
    if (!firstPass)
    {

        debug("II GLOB");
        IdentList *curr = list;

        do
        {
            debug("\tname: " + curr->val);

            auto e = SymMap.find(curr->val);
            if (e == SymMap.end())
            {
                log("Symbol not defined but declared global: " + curr->val);
                exit(EXIT_FAILURE);
            }

            e->second->isLocal = false;

            curr = curr->next;
        } while (curr);
    }

    delete list;
}

void Assembler::parseExtern(IdentList *list)
{
    if (!firstPass)
    {

        debug("II EXTR");
        IdentList *curr = list;

        do
        {
            debug("\tname: " + curr->val);

            auto e = addSymbol(curr->val);
            e->isLocal = false;
            e->section = EXTERNAL_SECTION;
            e->offset = 0;

            curr = curr->next;
        } while (curr);
    }

    delete list;
}

void Assembler::parseSection(string name)
{
    Assembler::getInstance().locationCounter = 0;
    //evidentiram tekucu sekciju
    Assembler::getInstance().currentSection++;

    if (firstPass)
    {
        debug("I SEC: " + name);

        //dodam sekciju u tabelu simbola
        auto s = addSymbol("." + name);
        //dodam novu sekciju
        Assembler::getInstance().Sections.push_back(new Section("." + name, s->id));
    }
    else
    {
        debug("II SEC: " + name);
    }
}

void Assembler::parseWord(IdentList *list)
{
    if (firstPass)
    {
        debug("I WORD");
        IdentList *curr = list;

        do
        {
            debug("\tname: " + curr->val);

            curr = curr->next;
            locationCounter += 2;
        } while (curr);
    }
    else
    {
        debug("II WORD");
        IdentList *curr = list;

        do
        {
            debug("\tname: " + curr->val);

            addNonRelativeValue(curr->val, 0);
            
            curr = curr->next;
            locationCounter += 2;
        } while (curr);
    }

    delete list;
}

void Assembler::parseWord(uint16_t word)
{
    if (!firstPass)
    {
        debug ("II WORD: " + to_string(word));

        addDataW(word);

        //dodaj word u data;
    }
    locationCounter += 2;
}

void Assembler::parseSkip(int literal)
{
    if (firstPass)
    {
        debug("I SKIP: " + to_string(literal));
    }
    else
    {
        for (int i = 0; i < literal; i++) {
            Sections[currentSection]->data.push_back(0);
        }
        debug("II SKIP: " + to_string(literal));
        //rel zapis u odnosu na sekciju?
        //inicijalizuj nulama u Data;
    }
    locationCounter += literal;
}

void Assembler::parseEqu(string ident, int literal)
{
    if (firstPass)
    {
        debug("I EQU " + ident + " = " + to_string(literal));
        SymTabEntry *curr = addSymbol(ident);
        curr->section = ABSOLUTE_SECTION;
        curr->offset = literal;
    }
    else
    {
        debug("II EQU " + ident + " = " + to_string(literal));
    }
}

void Assembler::parseEnd()
{
    if (firstPass)
    {
        debug("----------");
        secondPass();
    }
    else
    {
        Finish();
    }
}

void Assembler::parseLabel(string name)
{
    if (firstPass)
    {
        debug("I LAB: " + name);
        //ubacujem labelu u tabelu simbola
        addSymbol(name);
    }
}

Assembler::~Assembler()
{
    while (!SymTab.empty())
    {
        delete SymTab.back();
        SymTab.pop_back();
    }
    while (!Sections.empty())
    {
        delete Sections.back();
        Sections.pop_back();
    }
}

void Assembler::Finish()
{
    cout << "#tabela simbola" << endl;

    const char separator = ' ';
    const int width = 16;

    cout << left << setw(width) << setfill(separator) << "#naziv"
         << left << setw(width) << setfill(separator) << "sekcija"
         << left << setw(width) << setfill(separator) << "vr."
         << left << setw(width) << setfill(separator) << "Vez."
         << left << setw(width) << setfill(separator) << "R.b"
         << endl;

    for (auto i : SymTab)
    {
        string section;

        if (i->section < 0)
        {
            if (i->section == ABSOLUTE_SECTION)
            {
                section = "ABS";
            }
            else if (i->section == EXTERNAL_SECTION)
            {
                section = "EXT";
            }
        }
        else
        {
            section = Sections[i->section]->name;
        }

        cout << left << setw(width) << setfill(separator) << i->label
             << left << setw(width) << setfill(separator) << section
             << left << setw(width) << setfill(separator) << i->offset
             << left << setw(width) << setfill(separator) << (i->isLocal ? "local" : "global")
             << left << setw(width) << setfill(separator) << i->id
             << endl;
    }

    cout << endl;

    for (auto i: Sections) {
        if (i->rel.empty()) continue;

        cout << "#.rel" << i->name << endl;
        cout    << left << setw(width) << setfill(separator) << "#ofset"
                << left << setw(width) << setfill(separator) << "tip"
                << left << setw(width) << setfill(separator) << "redni_broj"
                << endl;

        for (auto j : i->rel) {
            cout << left << setw(width) << j->offset
                << left << setw(width) << RelTypesString[j->relType]
                << left << setw(width) << j->entry
                << endl;
        }

        cout << endl;
    }

    for (auto i: Sections) {
        if (i->data.empty()) continue;

        cout << "#" << i->name << endl;

        for (auto j : i->data) {
            printf("%.2X ", j);
        }

        cout << endl;
    }
}

void Assembler::push(int reg) {
    if (!firstPass) {
        debug("II PUSH R" + to_string(reg));
        uint32_t pushCode = 0xB00612;
        addData3B(pushCode + (reg << 12));
    }
    locationCounter += 3;
}

void Assembler::pop(int reg) {
    if (!firstPass) {
        debug("II POP R" + to_string(reg));
        uint32_t popCode = 0xA00642;
        addData3B(popCode + (reg << 12));
    }
    locationCounter += 3;
}

void Assembler::noaddr(uint8_t instr) {
    if (!firstPass) {
        debug("II NOADDR: " + to_string(instr));

        addDataB(instr);
    }
    locationCounter += 1;
}

void Assembler::tworeg(uint8_t instr, uint8_t rd, uint8_t rs) {
    if (!firstPass) {
        debug("II TWOREG: " + to_string(instr) + ": " + to_string(rd) + ", " + to_string(rs));

        uint16_t code = (instr << 8) + (rd << 4) + rs;
        addDataW(code);
    }
    locationCounter += 2;
}

void Assembler::jmp(uint8_t instr, Operand * op) {
    debug("II JMP: " + to_string(instr));

    if (firstPass) {
        locationCounter += op->getSize();
    }
    else {
        uint32_t code = (instr << 16) + op->calculate();

        addData3B(code);

        //op calculate ce da upise u memoriju 2B i uveca location counter ako treba
        locationCounter += 3;
    }
    delete op;
}

void Assembler::regop(uint8_t instr, uint8_t rd, Operand * op) {
    debug("II REGOP: " + to_string(instr));

    if (firstPass) {
        locationCounter += op->getSize();
    }
    else {
        uint32_t code = (instr << 16) + (rd << 12) + op->calculate();

        addData3B(code);

        //op calculate ce da upise u memoriju 2B i uveca location counter ako treba
        locationCounter += 3;
    }

    delete op;
}

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