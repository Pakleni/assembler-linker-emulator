#include "../inc/assembler.hpp"

using namespace std;

const int ABSOLUTE_SECTION = -1;

int SymTabEntry::idc = 0;
int locationCounter = 0;
int currentSection = -1;

bool firstPass = true;

void secondPass()
{
    locationCounter = 0;
    firstPass = false;
}

SymTabEntry* addSymbol(string label)
{
    SymTab.push_back(new SymTabEntry(label));

    return SymTab.back();
}
void onDirective()
{
    if (firstPass)
    {
        // labela
        onLabel("ime labele");
    }
    else
    {
        // .[global,extern] <lista>
        // foreach iz liste addSymbol
    }
}

void onSection(string name)
{
    locationCounter = 0;
    //dodam novu sekciju
    Sections.push_back(name);
    //evidentiram tekucu sekciju
    currentSection++;
    //dodam sekciju u tabelu simbola
    addSymbol(name);
}

void onLabel(string name)
{
    //ubacujem labelu u tabelu simbola
    addSymbol(name);
}

void onWord() {
    if (firstPass) {
        //lista simbola
        // foreach
        locationCounter+=2;

        //literal
        locationCounter+=2;
    }
    else {
        //zapravo upises sve vrednosti
    }
}

void onSkip() {
    if (firstPass) {
        //aaa skontacemo kasnije
        //locationCounter += neki broj;
    }
    else {
        //locationCounter += neki broj;
        //inicijalizuj nulama to cudo breJ;
    }
}

void onEqu() {
    SymTabEntry * curr =  addSymbol("ime simbola");
    curr->section = ABSOLUTE_SECTION;
    //curr->offset => value;
}