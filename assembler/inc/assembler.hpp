#include <stdio.h>
#include <string>
#include <vector>

class SymTabEntry
{
private:
    static int idc;

public:
    int id = idc++;

    std::string label;
    //dobija id trenutnog section-a
    int section = currentSection;
    //dobija trenutnu vrednost location counter-a
    int offset = locationCounter;
    //local u prvom prolazu
    bool isLocal = true;

    SymTabEntry(std::string _label) : label(_label) {}
};

class RelEntry
{
    enum RelTypes
    {
        PCRel,
        Absolute
    };
    int section;
    int offset;
    int addend;
    RelTypes relType;
};

class TextEntry
{
public:
    int section;
    int offset;
    int data;
};

int locationCounter;
int currentSection;
bool firstPass;

std::vector<SymTabEntry *> SymTab;
std::vector<RelEntry *> Rel;
std::vector<TextEntry *> Text;
std::vector<std::string> Sections;

void secondPass();