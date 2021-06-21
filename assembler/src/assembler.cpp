#include "../inc/assembler.hpp"
#include <iostream>

using namespace std;

const int ABSOLUTE_SECTION = -1;

int SymTabEntry::idc = 0;

void Assembler::secondPass()
{
    locationCounter = 0;
    firstPass = false;
}

SymTabEntry * Assembler::addSymbol(string label)
{
    SymTab.push_back(new SymTabEntry(label));

    return SymTab.back();
}

void Assembler::parseGlobal(IdentList* list) {
    if (firstPass) {
        cout << "I GLOB" << endl;

        delete list;
    }
    else {
        cout << "II GLOB" << endl;

        IdentList *curr = list;

        do {
            cout << "\t\tname: " << curr->val << endl;
            curr = curr->next;
        }
        while (curr);

        delete list;
    }
}

void Assembler::parseExtern(IdentList* list) {
    
}

void Assembler::parseSection(string name)
{
    if (firstPass) {
        cout << "I SEC | Name: " << name << endl;

        Assembler::getInstance().locationCounter = 0;
        //dodam novu sekciju
        Assembler::getInstance().Sections.push_back(name);
        //evidentiram tekucu sekciju
        Assembler::getInstance().currentSection++;
        //dodam sekciju u tabelu simbola
        addSymbol(name);
    }
    else {
        cout << "II SEC | Name: " << name << endl;
    }
}

void Assembler::parseWord(IdentList* list)
{
    if (firstPass)
    {
        //foreach
        // locationCounter += 2;
    }
    else
    {
        //zapravo upises sve vrednosti
    }
}

void Assembler::parseWord(int) {

}

void Assembler::parseSkip(int literal)
{
    locationCounter += literal;
    if (firstPass)
    {
        cout << "I SKIP | literal: " << literal << endl;
        //aaa skontacemo kasnije
    }
    else
    {
        cout << "II SKIP | literal: " << literal << endl;
        //inicijalizuj nulama to cudo breJ;
    }
}

void Assembler::parseEqu(string ident, int literal)
{
    if (firstPass) {
        cout << "I EQU | ident: " << ident << " | literal: " << literal << endl;
        SymTabEntry *curr = addSymbol(ident);
        curr->section = ABSOLUTE_SECTION;
    }
    else {
        cout << "II EQU | ident: " << ident << " | literal: " << literal << endl;
    }
}

void Assembler::parseEnd()
{
    if (firstPass)
    {
        cout << "----------" << endl;
        secondPass();
    }
    else
    {
    }
}

void Assembler::parseLabel(string name)
{
    if (firstPass) {
        cout << "I LAB | name: " << name << endl;
        //ubacujem labelu u tabelu simbola
        addSymbol(name);
    }
    else {
        cout << "II LAB | name: " << name << endl;
    }
}

Assembler::~Assembler()
{
    while (!SymTab.empty())
    {
        delete SymTab.back();
        SymTab.pop_back();
    }
    while (!Rel.empty())
    {
        delete Rel.back();
        Rel.pop_back();
    }
    while (!Data.empty())
    {
        delete Data.back();
        Data.pop_back();
    }
}