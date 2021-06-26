#include "../inc/printer.hpp"
#include "../inc/elf_structures.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

const string SYMTABSTRING = ".symtab";
const int HEADERS_BEFORE_SECTIONS = 3;

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

    stringstream ss;

    ss << "#tabela simbola" << endl;

    const char separator = ' ';
    const int width = 16;

    ss << left << setw(width) << setfill(separator) << "#naziv"
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

        ss << left << setw(width) << setfill(separator) << i->label
             << left << setw(width) << setfill(separator) << section
             << left << setw(width) << setfill(separator) << i->offset
             << left << setw(width) << setfill(separator) << (i->isLocal ? "local" : "global")
             << left << setw(width) << setfill(separator) << i->id
             << endl;
    }

    ss << endl;

    for (auto i : sections)
    {
        if (i->rel.empty())
            continue;

        ss << "#.rel" << i->name << endl;
        ss << left << setw(width) << setfill(separator) << "#ofset"
             << left << setw(width) << setfill(separator) << "tip"
             << left << setw(width) << setfill(separator) << "redni_broj"
             << endl;

        for (auto j : i->rel)
        {
            ss << left << setw(width) << j->offset
                 << left << setw(width) << RelTypesString[j->relType]
                 << left << setw(width) << j->entry
                 << endl;
        }

        ss << endl;
    }

    string out = ss.str();
    fprintf(file, "%s", out.c_str());

    for (auto i : sections)
    {
        if (i->data.empty())
            continue;

        fprintf(file, "#%s\n" , i->name.c_str() );

        for (auto j : i->data)
        {
            fprintf(file, "%.2X ", j);
        }

        fprintf(file, "\n");
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
    fprintf(file, "%s", strtab.c_str());
    writeB(0);
    c += strtab.size() + 1;

    strtab = ".strtab";
    fprintf(file, "%s", strtab.c_str());
    writeB(0);
    c += strtab.size() + 1;

    for (auto i : sections)
    {
        if (!i->rel.empty())
        {
            i->string_table_reli = c;
            string rel = ".rel";
            fprintf(file, "%s", rel.c_str());
            c += rel.size();
        }
        i->string_table_i = c;
        symtab[i->id]->string_table_i = c;
        fprintf(file, "%s", i->name.c_str());
        writeB(0);
        c += i->name.size() + 1;
    }

    fprintf(file, "%s", SYMTABSTRING.c_str());

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
        fprintf(file, "%s", i->label.c_str());
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