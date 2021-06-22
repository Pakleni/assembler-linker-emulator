#include "../inc/assembler.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

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

        //dodam novu sekciju
        Assembler::getInstance().Sections.push_back(new Section("." + name));
        //dodam sekciju u tabelu simbola
        addSymbol("." + name);
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

            //dodaj value u DATA

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

        uint8_t l = (word & 0xFF00 ) >> 8;
        uint8_t h =  word & 0x00FF;

        Sections[currentSection]->data.push_back(l);
        Sections[currentSection]->data.push_back(h);

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
        cout << "#.rel" << i->name << endl;
        cout    << left << setw(width) << setfill(separator) << "#ofset"
                << left << setw(width) << setfill(separator) << "tip"
                << left << setw(width) << setfill(separator) << "redni_broj"
                << endl;

        for (auto j : i->rel) {
            //TODO
        }

        cout << endl;
    }

    for (auto i: Sections) {
        cout << "#" << i->name << endl;

        for (auto j : i->data) {
            printf("%.2X ", j);
        }

        cout << endl;
    }
}