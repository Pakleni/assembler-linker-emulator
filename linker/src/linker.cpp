#include "../inc/linker.hpp"
#include <iostream>
#include <sstream>
// #include <fstream>
#include <string.h>

using namespace std;

void Linker::parsePlace(string s) {
    auto i = s.find("=");

    if (i == string::npos) { return; }

    string s_new = s.substr(i+1);
    
    i = s_new.find("@");

    if (i == string::npos) { 
        cout << "Error in -place: " << s << endl;
        return;
    }

    string s1 = s_new.substr(0, i);
    string s2 = s_new.substr(i+1);

    int x;   
    std::stringstream ss;
    ss << std::hex << s2;
    ss >> x;

    places.insert({s1, x});
}

Linker::~Linker() {

}

void Linker::start(string out) {
    if (mode == none) return;

    for (auto i: sources) {
        parseFile(i);
    }

    output(out);
}

uint32_t readDW(FILE * file){
    uint32_t ret = 0;
    char c;
    int d = 0;
    for (int i = 0; i < 4; i++) {
        c = fgetc(file);
        ret += (c << 8*(d++));
    }

    return ret;
}
uint16_t readW(FILE * file){
    uint16_t ret = 0;
    char c;
    int d = 0;
    for (int i = 0; i < 2; i++) {
        c = fgetc(file);
        ret += (c << 8*(d++));
    }

    return ret;
}

string readStr(FILE * file) {
    string str = "";

    char c= fgetc(file);

    while (c!= 0) {
        str += c;
        c = fgetc(file);
    }

    return str;
}

struct SectionHeader {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t addralign;
    uint32_t entsize;
};

struct SymTabEntry {
    uint32_t name;
    uint32_t value;
    uint32_t size;
    unsigned char info;
    unsigned char other;
    uint16_t shndx;
};

void Linker::parseFile(string name) {
    // ifstream file(name, ifstream::binary);
    FILE * file = fopen(name.c_str(), "rb");

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

    fseek(file, 32, SEEK_SET);
    int shoff = readDW(file);

    fseek(file, 48, SEEK_SET);
    int shnum = readW(file);
    int shstrndx = readW(file);

    fseek(file, shoff + sizeof(SectionHeader)*shstrndx, SEEK_SET);

    SectionHeader shstr;
    fread(&shstr, sizeof(SectionHeader), 1, file);

    SectionHeader * sh = new SectionHeader[shnum];

    fseek(file, shoff, SEEK_SET);
    for (int i = 0; i < shnum; i++) {
        fread(&(sh[i]), sizeof(SectionHeader), 1, file);
    }

    string * sh_names = new string[shnum];

    for (int i = 0; i < shnum; i++) {
        fseek(file, shstr.offset + sh[i].name, SEEK_SET);
        sh_names[i] = readStr(file);
    }

    SectionHeader * symtab = sh + (shnum - 1);

    int symbols_num = symtab->size / symtab->entsize;
    SymTabEntry * symbols = new SymTabEntry[symbols_num];

    fseek(file, symtab->offset, SEEK_SET);
    for (int i = 0; i< symbols_num; i++) {
        fread(&(symbols[i]), sizeof(SymTabEntry), 1, file);
    }

    SectionHeader * strtab = sh + symtab->link;
    string * sym_names = new string[symbols_num];
    for (int i = 0; i< symbols_num; i++) {
        fseek(file, strtab->offset + symbols[i].name, SEEK_SET);
        sym_names[i] = readStr(file);
    }

    //data sections
    //rel sections

    // Close the file
    fclose(file);
    delete[] sh;
    delete[] sh_names;
    delete[] symbols;
    delete[] sym_names;
    cout << "SUCC" << endl;
}

void Linker::output(string file) {

}
