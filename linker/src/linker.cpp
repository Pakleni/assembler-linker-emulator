#include "../inc/linker.hpp"
#include "../../assembler/inc/printer.hpp"
#include <iostream>
#include <sstream>

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
    while (!files.empty())
    {
        delete files.back();
        files.pop_back();
    }
}

void Linker::start(string out) {
    if (mode == none) return;

    for (auto i: sources) {
        parseFile(i);
    }

    output(out);
}

void Linker::parseFile(string name) {
    ELFFile * elf = Reader(name).read();
    files.push_back(elf);
    
    FILE * file = fopen("out/test.bin", "wb");
    
    Printer(elf->sections,
            elf->symtab,
            stdout).HumanPrint();

    fclose(file);
}

void Linker::output(string file) {

}
