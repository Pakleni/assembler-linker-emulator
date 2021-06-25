#include "../inc/assembler.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

const int ELF_SIZE = 0x34;
const int REL_SIZE = 8;
const int SYM_SIZE = 16;
const int SHENTRY_SIZE = 0x28;
const string SYMTABSTRING = ".symtab";
const int HEADERS_BEFORE_SECTIONS = 3;

void writeB(uint8_t b) {
    cout << char(b);
}

void writeW(uint16_t word) {
    uint8_t h = (word & 0xFF00 ) >> 8;
    uint8_t l =  word & 0x00FF;

    writeB(l);
    writeB(h);
}

void writeDW(uint32_t dw) {
    uint8_t l1 = (dw & 0xFF000000 ) >> 24;
    uint8_t l2 = (dw & 0x00FF0000 ) >> 16;
    uint8_t l3 = (dw & 0x0000FF00 ) >> 8;
    uint8_t l4 =  dw & 0x000000FF;

    writeB(l4);
    writeB(l3);
    writeB(l2);
    writeB(l1);
}

void WriteSection(Section * i) {
    //indeks u tabeli stringova
    writeDW(i->string_table_i);
    //type PROGBITS || NOBITS
    writeDW(i->data.size() > 0 ? 1 : 8);
    //flags exec, alloc, write
    writeDW(7);
    //adresa
    writeDW(0);
    //ofset
    writeDW(i->offset);
    //size
    writeDW(i->data.size());
    //link
    writeDW(0);
    //info
    writeDW(0);
    //align
    writeDW(4);
    //velicina ulaza
    writeDW(0);
}

int SHSTRTABSize() {
    int c = 1;

    string strtab = ".shstrtab";
    c += strtab.size() + 1;

    strtab = ".strtab";
    c += strtab.size() + 1;

    for (auto i: Assembler::getInstance().Sections) {
        if (!i->rel.empty()) {
            string rel = ".rel";
            c += rel.size();
        }
        c+= i->name.size() + 1;
    }
    
    c += SYMTABSTRING.size() + 1;

    return c;
}

int STRTABSize() {
    int c = 0;

    for (auto i: Assembler::getInstance().SymTab) {
        i->string_table_i = c;
        c+= i->label.size() + 1;
    }

    return c;
}

int CalculateSHOFF() {
    int c = ELF_SIZE;
    c += SHSTRTABSize();
    c += STRTABSize();
    c += Assembler::getInstance().SymTab.size() * SYM_SIZE;

    for (auto i: Assembler::getInstance().Sections) {
        c+= i->data.size();
        c+= i->rel.size() * REL_SIZE;
    }

    return c;
}

void ELFHeader() {
    char header[] = {
                    //magic
                    0x7f, 'E', 'L', 'F',
                    //class
                    1,
                    //data
                    1,
                    //version
                    1,
                    0
                    };
    
    cout << header;
    //padding
    writeB(0);
    writeDW(0);
    writeDW(0);
    //type
    writeW(1);
    //machine
    writeW(0);
    //version
    writeDW(1);
    //entry
    writeDW(0);
    //phoff
    writeDW(0);
    //shoff
    writeDW(CalculateSHOFF());
    //flags
    writeDW(0);
    //elf header size
    writeW(ELF_SIZE);
    //program header entry size
    writeW(0);
    //program header num
    writeW(0);
    //section header entry size
    writeW(SHENTRY_SIZE);
    //sec h num
    int num = HEADERS_BEFORE_SECTIONS + 1;
    for(auto i: Assembler::getInstance().Sections) {
        ++num;
        if (i->rel.size() > 0) ++num;
    }
    
    writeW(num);
    //shstrndx bice uvek 1... lakse tako
    writeW(1);
}

int SHSTRTable() {
    writeB(0); //undef
    int c = 1;

    string strtab = ".shstrtab";
    cout << strtab;
    writeB(0);
    c += strtab.size() + 1;

    strtab = ".strtab";
    cout << strtab;
    writeB(0);
    c += strtab.size() + 1;

    for (auto i: Assembler::getInstance().Sections) {
        if (!i->rel.empty()) {
            i->string_table_reli = c;
            string rel = ".rel";
            cout << rel;
            c += rel.size();
        }
        i->string_table_i = c;
        Assembler::getInstance().SymTab[i->id]->string_table_i = c;
        cout << i->name;
        writeB(0);
        c+= i->name.size() + 1;
    }

    cout << SYMTABSTRING;
    writeB(0);
    c += SYMTABSTRING.size() + 1;

    return c;
}

int STRTable() {
    int c = 0;

    for (auto i: Assembler::getInstance().SymTab) {
        i->string_table_i = c;
        cout << i->label;
        writeB(0);
        c+= i->label.size() + 1;
    }

    return c;
}

void SHTable(int shstrttabsize, int symtabsize, int strtabsize) {
    //UNDEF section
    for (int i = 0; i < 10; i++) writeDW(0);
    
    //string header table
    //indeks u tabeli stringova
    writeDW(1);
    //type STRTAB
    writeDW(3);
    //flags
    writeDW(0);
    //adresa
    writeDW(0);
    //ofset odma posle header
    writeDW(ELF_SIZE);
    //size
    writeDW(shstrttabsize);
    //link
    writeDW(0);
    //info
    writeDW(0);
    //align
    writeDW(1);
    //velicina ulaza
    writeDW(0);

    //string table
    //indeks u tabeli stringova
    writeDW(11);
    //type STRTAB
    writeDW(3);
    //flags
    writeDW(0);
    //adresa
    writeDW(0);
    //ofset odma posle header
    writeDW(ELF_SIZE + shstrttabsize);
    //size
    writeDW(strtabsize);
    //link
    writeDW(0);
    //info
    writeDW(0);
    //align
    writeDW(1);
    //velicina ulaza
    writeDW(0);

    for (auto i: Assembler::getInstance().Sections) {
        WriteSection(i);
    }

    int num = HEADERS_BEFORE_SECTIONS;
    for(auto i: Assembler::getInstance().Sections) {
        ++num;
        if (i->rel.size() > 0) ++num;
    }

    for (auto i: Assembler::getInstance().Sections) {
        if (i->rel.empty()) continue;

        //indeks u tabeli stringova
        writeDW(i->string_table_reli);
        //type REL
        writeDW(9);
        //flags exec, alloc, write
        writeDW(0);
        //adresa
        writeDW(0);
        //ofset
        writeDW(i->rel_offset);
        //size
        writeDW(i->rel.size()*REL_SIZE);
        //link
        writeDW(num);
        //info
        writeDW(Assembler::getInstance().SymTab[i->id]->section + HEADERS_BEFORE_SECTIONS);
        //align
        writeDW(4);
        //velicina ulaza
        writeDW(REL_SIZE);
    }

    //symtab table
    //indeks u tabeli stringova [poslednji]
    writeDW(shstrttabsize - SYMTABSTRING.size() - 1);
    //type SYMTAB
    writeDW(2);
    //flags
    writeDW(0);
    //adresa
    writeDW(0);
    //ofset odma posle shstr
    writeDW(ELF_SIZE + shstrttabsize + strtabsize);
    //size
    writeDW(symtabsize);
    //link
    writeDW(2);
    //info
    writeDW(0);
    //align
    writeDW(4);
    //velicina ulaza
    writeDW(0x10);
}

void PrintSym(SymTabEntry * i) {
    writeDW(i->string_table_i);
    writeDW(i->offset);
    //size?
    writeDW(0);
    writeB((i->isLocal? 0x00 : 0x10) + (i->isSection? 0x3 : 0x1));
    writeB(0);
    
    if (i->section == EXTERNAL_SECTION) {
        writeW(0);
    }
    else if (i->section == ABSOLUTE_SECTION) {
        writeW(0xFFF1);
    }
    else {
        writeW(i->section + HEADERS_BEFORE_SECTIONS);
    }
}

int SYMTable() {
    int c = 0;

    for (auto i: Assembler::getInstance().SymTab) {
        
        PrintSym(i);
        c+=SYM_SIZE;
    }

    return c;
}

void DataSections(int* offset) {

    for (auto i: Assembler::getInstance().Sections) {
        i->offset = *offset;

        for (auto j: i->data) {
            writeB(j);
            (*offset)++;
        }

    }
}

void RelSections(int* offset)  {
    for (auto i: Assembler::getInstance().Sections) {
        i->rel_offset = *offset;
        for (auto j: i->rel) {
            writeDW(j->offset);

            int relType = j->relType == RelEntry::R_16 ? 1 : 2;

            writeDW((j->entry << 8) + relType);

            (*offset) += REL_SIZE;
        }
    }
}

void Printer::Print(){

    ELFHeader();
    int c = ELF_SIZE;
    
    int shrstrssize = SHSTRTable();
    c+= shrstrssize;

    int strtabsize = STRTable();
    c+= strtabsize;

    int symtabsize = SYMTable();
    c+= symtabsize;

    DataSections(&c);

    RelSections(&c);

    SHTable(shrstrssize, symtabsize, strtabsize);
}

void Printer::HumanPrint() {

    const char* RelTypesString[] = {"R_16", "R_PC16"};

    cout << "#tabela simbola" << endl;

    const char separator = ' ';
    const int width = 16;

    cout << left << setw(width) << setfill(separator) << "#naziv"
         << left << setw(width) << setfill(separator) << "sekcija"
         << left << setw(width) << setfill(separator) << "vr."
         << left << setw(width) << setfill(separator) << "Vez."
         << left << setw(width) << setfill(separator) << "R.b"
         << endl;

    for (auto i : Assembler::getInstance().SymTab)
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
            section = Assembler::getInstance().Sections[i->section]->name;
        }

        cout << left << setw(width) << setfill(separator) << i->label
             << left << setw(width) << setfill(separator) << section
             << left << setw(width) << setfill(separator) << i->offset
             << left << setw(width) << setfill(separator) << (i->isLocal ? "local" : "global")
             << left << setw(width) << setfill(separator) << i->id
             << endl;
    }

    cout << endl;

    for (auto i: Assembler::getInstance().Sections) {
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

    for (auto i: Assembler::getInstance().Sections) {
        if (i->data.empty()) continue;

        cout << "#" << i->name << endl;

        for (auto j : i->data) {
            printf("%.2X ", j);
        }

        cout << endl;
    }
}