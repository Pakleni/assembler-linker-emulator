#include "../inc/printer.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

const string SYMTABSTRING = ".symtab";
const int HEADERS_BEFORE_SECTIONS = 3;

struct ELFSHeader
{
    uint32_t name = 0;
    uint32_t type = 0;
    uint32_t flags = 0;
    uint32_t addr = 0;
    uint32_t offset = 0;
    uint32_t size = 0;
    uint32_t link = 0;
    uint32_t info = 0;
    uint32_t addralign = 0;
    uint32_t entsize = 0;

    ELFSHeader() {}
    ELFSHeader(uint32_t _name,
               uint32_t _type,
               uint32_t _flags,
               uint32_t _offset,
               uint32_t _size,
               uint32_t _link,
               uint32_t _info,
               uint32_t _addralign,
               uint32_t _entsize) : name(_name),
                                    type(_type),
                                    flags(_flags),
                                    offset(_offset),
                                    size(_size),
                                    link(_link),
                                    info(_info),
                                    addralign(_addralign),
                                    entsize(_entsize) {}

    void write(FILE * file) { fwrite(this, sizeof(*this), 1, file); }
};

struct ELFSTEntry
{
    uint32_t name;
    uint32_t value;
    uint32_t size = 0;
    unsigned char info;
    unsigned char other = 0;
    uint16_t shndx;

    ELFSTEntry(uint32_t _name,
               uint32_t _value,
               unsigned char _info,
               uint16_t _shndx) : name(_name),
                                  value(_value),
                                  info(_info),
                                  shndx(_shndx) {}

    void write(FILE * file) { fwrite(this, sizeof(*this), 1, file); }
};

struct ELFHeader
{
    uint8_t M1 = 0x7f;
    uint8_t M2 = 'E';
    uint8_t M3 = 'L';
    uint8_t M4 = 'F';
    uint8_t file_class = 1;
    uint8_t data = 1;
    uint8_t version = 1;
    uint8_t padd1 = 0;
    uint32_t padd2 = 0;
    uint32_t padd3 = 0;
    uint16_t type = 1;
    uint16_t machine = 0;
    uint32_t version2 = 1;
    uint32_t entry = 0;
    uint32_t phoff = 0;
    uint32_t shoff;
    uint32_t flags = 0;
    uint16_t size = sizeof(ELFHeader);
    uint16_t phentsz = 0;
    uint16_t phnum = 0;
    uint16_t shentsize = sizeof(ELFSHeader);
    uint16_t shndx;
    uint16_t shstrndx = 1;

    ELFHeader(uint32_t _shoff, uint16_t _shndx) : shoff(_shoff), shndx(_shndx) {}
    void write(FILE * file) { fwrite(this, sizeof(*this), 1, file); }
};

struct ELFRelEntry
{
    uint32_t offset;
    uint32_t info;

    ELFRelEntry(uint32_t _of, uint32_t _in) : offset(_of), info(_in) {}
    void write(FILE * file) { fwrite(this, sizeof(*this), 1, file); }
};

void Printer::Print()
{
    printELFHeader();
    offset = sizeof(ELFHeader);

    int shrstrssize = printSHStrTable();
    offset += shrstrssize;

    int strtabsize = printStrTable();
    offset += strtabsize;

    int symtabsize = printSymTable();
    offset += symtabsize;

    printDataSections();

    printRelSections();

    printSHTable(shrstrssize, symtabsize, strtabsize);
}

void Printer::HumanPrint()
{

    const char *RelTypesString[] = {"R_16", "R_PC16"};

    cout << "#tabela simbola" << endl;

    const char separator = ' ';
    const int width = 16;

    cout << left << setw(width) << setfill(separator) << "#naziv"
         << left << setw(width) << setfill(separator) << "sekcija"
         << left << setw(width) << setfill(separator) << "vr."
         << left << setw(width) << setfill(separator) << "Vez."
         << left << setw(width) << setfill(separator) << "R.b"
         << endl;

    for (auto i : symtab)
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
            section = sections[i->section]->name;
        }

        cout << left << setw(width) << setfill(separator) << i->label
             << left << setw(width) << setfill(separator) << section
             << left << setw(width) << setfill(separator) << i->offset
             << left << setw(width) << setfill(separator) << (i->isLocal ? "local" : "global")
             << left << setw(width) << setfill(separator) << i->id
             << endl;
    }

    cout << endl;

    for (auto i : sections)
    {
        if (i->rel.empty())
            continue;

        cout << "#.rel" << i->name << endl;
        cout << left << setw(width) << setfill(separator) << "#ofset"
             << left << setw(width) << setfill(separator) << "tip"
             << left << setw(width) << setfill(separator) << "redni_broj"
             << endl;

        for (auto j : i->rel)
        {
            cout << left << setw(width) << j->offset
                 << left << setw(width) << RelTypesString[j->relType]
                 << left << setw(width) << j->entry
                 << endl;
        }

        cout << endl;
    }

    for (auto i : sections)
    {
        if (i->data.empty())
            continue;

        cout << "#" << i->name << endl;

        for (auto j : i->data)
        {
            printf("%.2X ", j);
        }

        cout << endl;
    }
}

int Printer::getSHStrTableSize()
{
    int c = 1;

    string strtab = ".shstrtab";
    c += strtab.size() + 1;

    strtab = ".strtab";
    c += strtab.size() + 1;

    for (auto i : sections)
    {
        if (!i->rel.empty())
        {
            string rel = ".rel";
            c += rel.size();
        }
        c += i->name.size() + 1;
    }

    c += SYMTABSTRING.size() + 1;

    return c;
}

int Printer::getStrTableSize()
{
    int c = 0;

    for (auto i : symtab)
    {
        i->string_table_i = c;
        c += i->label.size() + 1;
    }

    return c;
}

int Printer::getSHOffset()
{
    int c = sizeof(ELFHeader);
    c += getSHStrTableSize();
    c += getStrTableSize();
    c += symtab.size() * sizeof(ELFSTEntry);

    for (auto i : sections)
    {
        c += i->data.size();
        c += i->rel.size() * sizeof(ELFRelEntry);
    }

    return c;
}

void Printer::printELFHeader()
{
    //sec h num
    int num = HEADERS_BEFORE_SECTIONS + 1;
    for (auto i : sections)
    {
        ++num;
        if (i->rel.size() > 0)
            ++num;
    }

    ELFHeader(getSHOffset(), num).write(file);
}

int Printer::printSHStrTable()
{
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

    for (auto i : sections)
    {
        if (!i->rel.empty())
        {
            i->string_table_reli = c;
            string rel = ".rel";
            cout << rel;
            c += rel.size();
        }
        i->string_table_i = c;
        symtab[i->id]->string_table_i = c;
        cout << i->name;
        writeB(0);
        c += i->name.size() + 1;
    }

    cout << SYMTABSTRING;
    writeB(0);
    c += SYMTABSTRING.size() + 1;

    return c;
}

int Printer::printStrTable()
{
    int c = 0;

    for (auto i : symtab)
    {
        i->string_table_i = c;
        cout << i->label;
        writeB(0);
        c += i->label.size() + 1;
    }

    return c;
}

int Printer::printSymTable()
{
    int c = 0;

    for (auto i : symtab)
    {

        uint16_t shndx;

        if (i->section == EXTERNAL_SECTION)
        {
            shndx = 0;
        }
        else if (i->section == ABSOLUTE_SECTION)
        {
            shndx = 0xFFF1;
        }
        else
        {
            shndx = i->section + HEADERS_BEFORE_SECTIONS;
        }

        ELFSTEntry(i->string_table_i, i->offset, (i->isLocal ? 0x00 : 0x10) + (i->isSection ? 0x3 : 0x1), shndx).write(file);

        c += sizeof(ELFSTEntry);
    }

    return c;
}

void Printer::printDataSections()
{
    for (auto i : sections)
    {
        i->offset = offset;

        for (auto j : i->data)
        {
            writeB(j);
            offset++;
        }
    }
}

void Printer::printRelSections()
{
    for (auto i : sections)
    {
        i->rel_offset = offset;
        for (auto j : i->rel)
        {
            int relType = j->relType == RelEntry::R_16 ? 1 : 2;

            ELFRelEntry(j->offset, (j->entry << 8) + relType).write(file);
            offset += sizeof(ELFRelEntry);
        }
    }
}

void Printer::printSHTable(int shstrttabsize, int symtabsize, int strtabsize)
{
    ELFSHeader().write(file);

    ELFSHeader(1,
               3,
               0,
               sizeof(ELFHeader),
               shstrttabsize,
               0,
               0,
               1,
               0)
        .write(file);

    ELFSHeader(11,
               3,
               0,
               sizeof(ELFHeader) + shstrttabsize,
               strtabsize,
               0,
               0,
               1,
               0)
        .write(file);

    for (auto i : sections)
    {
        ELFSHeader(i->string_table_i,
                   (i->data.size() > 0 ? 1 : 8),
                   7,
                   i->offset,
                   i->data.size(),
                   0,
                   0,
                   4,
                   0)
            .write(file);
    }

    int strtab_index = HEADERS_BEFORE_SECTIONS;
    for (auto i : sections)
    {
        ++strtab_index;
        if (i->rel.size() > 0)
            ++strtab_index;
    }

    for (auto i : sections)
    {
        if (i->rel.empty())
            continue;

        ELFSHeader(i->string_table_reli,
                   9,
                   0,
                   i->rel_offset,
                   i->rel.size() * sizeof(ELFRelEntry),
                   strtab_index,
                   symtab[i->id]->section + HEADERS_BEFORE_SECTIONS,
                   4,
                   sizeof(ELFRelEntry))
            .write(file);
    }

    ELFSHeader(shstrttabsize - SYMTABSTRING.size() - 1,
               2,
               0,
               sizeof(ELFHeader) + shstrttabsize + strtabsize,
               symtabsize,
               2,
               0,
               4,
               sizeof(ELFSTEntry))
        .write(file);
}