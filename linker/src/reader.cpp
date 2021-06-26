#include "../inc/reader.hpp"
#include <iostream>
#include <string.h>

using namespace std;

string readStr(FILE * file) {
    string str = "";

    char c= fgetc(file);

    while (c!= 0 && c!= EOF) {
        str += c;
        c = fgetc(file);
    }
    if (c == EOF) {
        cout << "End of file reached!" << endl;
        exit(1);
    }

    return str;
}

const int HEADERS_BEFORE_SECTIONS = 3;

void ELFFile::addSymbol(string label, ELFSTEntry * st) {
    int section = st->shndx;

    if (section == 0) {
        section = EXTERNAL_SECTION;
    }
    else if (section == 0xfff1) {
        section = ABSOLUTE_SECTION;
    }
    else {
        section -= HEADERS_BEFORE_SECTIONS;
    }

    SymTabEntry * add = new SymTabEntry(label, section, st->value);

    add->id = symtab.size();

    switch ((st->info & 0xF0) >> 4) {
        case (1):
        add->isLocal = false;
        break;
        default:
        add->isLocal = true;
        break;
    }

    switch (st->info & 0x0F) {
        case (3):
        add->isSection = true;
        sections[add->section]->id = add->id;
        break;
        default:
        add->isSection = false;
        break;
    }

    symtab.push_back(add);
}

void ELFFile::addSection(string label) {
    
    Section * add = new Section(label, -1);

    sections.push_back(add);
}

void ELFFile::addRel(ELFRelEntry * entry, ELFSHeader * header) {
    int section = header->info - HEADERS_BEFORE_SECTIONS;

    int offset = entry->offset;
    RelEntry::RelTypes type = (entry->info & 0x000000FF) == 1 ? RelEntry::R_16 : RelEntry::R_PC16;
    int entry_id = (entry->info & 0xFFFFFF00) >> 8;
    RelEntry * add = new RelEntry(offset,type, entry_id);

    sections[section]->rel.push_back(add);
}

ELFFile::~ELFFile() {
    while (!symtab.empty())
    {
        delete symtab.back();
        symtab.pop_back();
    }
    while (!sections.empty())
    {
        delete sections.back();
        sections.pop_back();
    }
}

ELFFile * Reader::read() {
    
    FILE * file = fopen(filename.c_str(), "rb");

    char header[] = {0x7f, 'E', 'L', 'F' , '\0'};
    char read[5];
    read[4] = '\0';

    for (int i = 0; i < 4; i++) {
        read[i] = fgetc(file);
    }

    if (strcmp(header, read) != 0) {
        cout << "Input not in correct format" << endl;
        exit(EXIT_FAILURE);
    }

    ELFFile * elf = new ELFFile();

    fseek(file, 32, SEEK_SET);
    uint32_t shoff;
    fread(&shoff, sizeof(shoff), 1, file);

    fseek(file, 48, SEEK_SET);
    uint16_t shnum;
    fread(&shnum, sizeof(shnum), 1, file);    
    uint16_t shstrndx;
    fread(&shstrndx, sizeof(shstrndx), 1, file);    

    fseek(file, shoff + sizeof(ELFSHeader)*shstrndx, SEEK_SET);

    ELFSHeader shstr;
    fread(&shstr, sizeof(ELFSHeader), 1, file);

    ELFSHeader * sh = new ELFSHeader[shnum];

    fseek(file, shoff, SEEK_SET);
    for (int i = 0; i < shnum; i++) {
        fread(&(sh[i]), sizeof(ELFSHeader), 1, file);
    }

    //sections
    string * sh_names = new string[shnum];

    for (int i = 0; i < shnum; i++) {
        fseek(file, shstr.offset + sh[i].name, SEEK_SET);
        sh_names[i] = readStr(file);

        if (sh[i].type == 1 || sh[i].type == 8) {
            elf->addSection(sh_names[i]);
        }
    }

    //symbols
    ELFSHeader * symtab = sh + (shnum - 1);

    int symbols_num = symtab->size / symtab->entsize;
    ELFSTEntry * symbols = new ELFSTEntry[symbols_num];

    fseek(file, symtab->offset, SEEK_SET);
    for (int i = 0; i< symbols_num; i++) {
        fread(&(symbols[i]), sizeof(ELFSTEntry), 1, file);
    }

    ELFSHeader * strtab = sh + symtab->link;
    string * sym_names = new string[symbols_num];
    for (int i = 0; i< symbols_num; i++) {
        fseek(file, strtab->offset + symbols[i].name, SEEK_SET);
        sym_names[i] = readStr(file);
        elf->addSymbol(sym_names[i], symbols + i);
    }

    //data sections
    for (int i = 0; i < shnum; i++) {
        if (sh[i].type == 1) {
            fseek(file, sh[i].offset, SEEK_SET);
            for (int j = 0; j < sh[i].size; j++) {
                elf->sections[i-HEADERS_BEFORE_SECTIONS]->data.push_back(fgetc(file));
            }
        }
    }
    //rel sections
    for (int i = 0; i < shnum; i++) {
        if (sh[i].type == 9) {
            fseek(file, sh[i].offset, SEEK_SET);
            for (int j = 0; j < sh[i].size/sh[i].entsize; j++) {
                
                ELFRelEntry curr;
                fread(&curr, sizeof(ELFRelEntry), 1, file);
                elf->addRel(&curr, sh + i);
            }
        }
    }

    // Close the file
    fclose(file);
    delete[] sh;
    delete[] sh_names;
    delete[] symbols;
    delete[] sym_names;

    return elf;
}