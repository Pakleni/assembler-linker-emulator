#include "../inc/assembler.hpp"
#include "../inc/printer.hpp"
#include <iostream>

using namespace std;

void log(string c)
{
    fprintf(stderr, "%s", (c + "\n").c_str());
}

void debug(string c)
{
    if (DEBUG)
        log(c);
}

void Assembler::addDataB(uint8_t byte)
{
    Sections[currentSection]->data.push_back(byte);
}

void Assembler::addDataW(uint16_t word)
{
    uint8_t h = (word & 0xFF00) >> 8;
    uint8_t l = word & 0x00FF;

    addDataB(l);
    addDataB(h);
}

void Assembler::addData3B(uint32_t word)
{
    uint8_t l1 = (word & 0x00FF0000) >> 16;
    uint8_t l2 = (word & 0x0000FF00) >> 8;
    uint8_t l3 = word & 0x000000FF;

    addDataB(l1);
    addDataB(l2);
    addDataB(l3);
}

void Assembler::addNonRelativeValue(string label, int offset)
{
    auto s = SymMap.find(label);
    SymTabEntry *sym;
    int id;

    if (s == SymMap.end())
    {
        sym = addSymbol(label);
        sym->isLocal = false;
        sym->offset = 0;
        sym->section = EXTERNAL_SECTION;
    }
    else
    {
        sym = s->second;
    }

    if (sym->section == ABSOLUTE_SECTION)
    {
        addDataW(sym->offset);
    }
    else
    {
        if (!sym->isLocal)
        {
            addDataW(0);
            id = sym->id;
        }
        else
        {
            addDataW(sym->offset); //addend
            id = Sections[sym->section]->id;
        }

        Sections[currentSection]->rel.push_back(new RelEntry(locationCounter + offset, RelEntry::R_16, id));
    }
}

void Assembler::addRelativeValue(string label, int offset)
{
    auto s = SymMap.find(label);
    SymTabEntry *sym;
    int id;

    if (s == SymMap.end())
    {
        sym = addSymbol(label);
        sym->isLocal = false;
        sym->offset = 0;
        sym->section = EXTERNAL_SECTION;
    }
    else
    {
        sym = s->second;
    }

    if (sym->section == ABSOLUTE_SECTION)
    {
        //nema smisla
        log("Syntax error: You are trying to relative jump to an absolute position");
        exit(EXIT_FAILURE);
    }
    else if (sym->section == currentSection)
    {
        addDataW(sym->offset - locationCounter - 2); //relative jump
    }
    else
    {

        if (!sym->isLocal)
        {
            addDataW(-2);
            id = sym->id;
        }
        else
        {
            addDataW(sym->offset - 2); //addend
            id = Sections[sym->section]->id;
        }

        Sections[currentSection]->rel.push_back(new RelEntry(locationCounter + offset, RelEntry::R_PC16, id));
    }
}

void Assembler::secondPass()
{
    locationCounter = 0;
    firstPass = false;
    currentSection = -1;
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
        s->isSection = true;
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
        debug("II WORD: " + to_string(word));

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
        for (int i = 0; i < literal; i++)
        {
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

void Assembler::push(int reg)
{
    if (!firstPass)
    {
        debug("II PUSH R" + to_string(reg));
        uint32_t pushCode = 0xB00612;
        addData3B(pushCode + (reg << 12));
    }
    locationCounter += 3;
}

void Assembler::pop(int reg)
{
    if (!firstPass)
    {
        debug("II POP R" + to_string(reg));
        uint32_t popCode = 0xA00642;
        addData3B(popCode + (reg << 12));
    }
    locationCounter += 3;
}

void Assembler::noaddr(uint8_t instr)
{
    if (!firstPass)
    {
        debug("II NOADDR: " + to_string(instr));

        addDataB(instr);
    }
    locationCounter += 1;
}

void Assembler::tworeg(uint8_t instr, uint8_t rd, uint8_t rs)
{
    if (!firstPass)
    {
        debug("II TWOREG: " + to_string(instr) + ": " + to_string(rd) + ", " + to_string(rs));

        uint16_t code = (instr << 8) + (rd << 4) + rs;
        uint8_t h = (code & 0xFF00) >> 8;
        uint8_t l = code & 0x00FF;

        addDataB(h);
        addDataB(l);
    }
    locationCounter += 2;
}

void Assembler::jmp(uint8_t instr, Operand *op)
{
    debug("II JMP: " + to_string(instr));

    if (firstPass)
    {
        locationCounter += op->getSize();
    }
    else
    {
        uint32_t code = (instr << 16) + op->calculate();

        addData3B(code);
        locationCounter += 3;
        op->add();
    }
    delete op;
}

void Assembler::regop(uint8_t instr, uint8_t rd, Operand *op)
{
    debug("II REGOP: " + to_string(instr));

    if (firstPass)
    {
        locationCounter += op->getSize();
    }
    else
    {
        uint32_t code = (instr << 16) + (rd << 12) + op->calculate();

        addData3B(code);

        locationCounter += 3;
        op->add();
    }

    delete op;
}

void Assembler::Finish()
{
    FILE *file = fopen(out.c_str(), "w+");
    Printer(Sections, SymTab, file).HumanPrint();

    freopen((out + ".bin").c_str(), "w+", file);
    Printer(Sections, SymTab, file).Print();
}

SymTabEntry *Assembler::addSymbol(string label)
{
    if (SymMap.count(label))
    {
        log("Symbol with name: " + label + " already exists!");
        exit(EXIT_FAILURE);
    }

    SymTabEntry *c = new SymTabEntry(label, currentSection, locationCounter);

    SymMap.insert({label, c});
    SymTab.push_back(c);

    return c;
}