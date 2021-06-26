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

void Linker::resolve(ELFFile * f) {
    for (auto i: f->symtab) {
        if (!i->isSection && !i->isLocal && i->section != EXTERNAL_SECTION) {
            if (resolved.find(i->label) != resolved.end()) {
                cout << "Symbol " << i->label << " defined more than once." << endl;
                exit(1);
            }

            SymTabEntry * add = new SymTabEntry(*i);
            add->id = symtab.size();

            symtab.push_back(add);
            resolved.insert({i->label, {add, f}});

            auto pos = unresolved.find(i->label);
            if (pos != unresolved.end()) {
                delete pos->second;
                unresolved.erase(pos);
            }
        }
    }
}

void Linker::addUnresolved(ELFFile * f) {
    for (auto i: f->symtab) {
        if (i->section == EXTERNAL_SECTION) {
            unresolved.insert({i->label, new SymTabEntry(*i)});
        }
    }
}

void Linker::start(string out) {
    if (mode == none) return;

    for (auto i: sources) {
        parseFile(i);
    }
    
    for(ELFFile * f : files) {
        //f-ovim defined, razresi unres slevo
        resolve(f);
        addUnresolved(f);
    }
    if (unresolved.size() > 0) {
        if (mode == Mode::hex) {
            cout << "Theres still unresolved symbols!" << endl;
            exit(1);
        }

        for (auto i: unresolved) {
            auto sym = i.second;
            sym->id = symtab.size();
            symtab.push_back(sym);
            resolved.insert({sym->label, {sym, nullptr}});
        }

    };

    //add all sections to sections and section_map and symtab
    for (ELFFile * f : files) {
        for(auto i: f->sections) {
            auto sec = section_map.find(i->name);

            Section * section;

            if (sec == section_map.end()) {
                //create entry for section
                SymTabEntry * sect = new SymTabEntry(i->name, sections.size(), 0);
                sect->isSection = true;
                sect->id = symtab.size();
                symtab.push_back(sect);
                resolved.insert({i->name, {sect, nullptr}});
                //add section to list
                section = new Section(i->name, sect->id);
                sections.push_back(section);
                section_map.insert({section->name, section});

            }
            else {
                section = sec->second;
            }

            //offset of my section
            f->symtab[i->id]->offset = section->data.size();

            for (auto d: i->data) {
                section->data.push_back(d);
            }
        }
    }

    //generate rel entries
    for (ELFFile * f : files) {
        for(auto i: f->sections) {
            auto sec = section_map.find(i->name);

            Section * section = sec->second;

            int off = f->symtab[i->id]->offset;

            for (auto r: i->rel) {
                string name = f->symtab[r->entry]->label;
                int id = resolved.find(name)->second.first->id;
                RelEntry * rel = new RelEntry(r->offset + off, r->relType, id);

                section->rel.push_back(rel);
            }
        }
    }

    //resolve multiple same sections
    for (auto i: resolved) {
        auto sym = i.second.first;

        if (sym->section != ABSOLUTE_SECTION) {
            auto f = i.second.second;
            if (f == nullptr) continue; //sekcija je ili undef
            auto old_section = f->sections[sym->section];
            //append offset
            sym->offset += f->symtab[old_section->id]->offset;
            //get id of new section
            sym->section = symtab[section_map.find(old_section->name)->second->id]->section;
        }
    }

    if (mode == Mode::hex) {
        int currentLocation = 0xFF00;
        //locations of sections
        for (int i = 0; i< sections.size(); i++) {
            auto s = sections[i];

            auto c = places.find(s->name.substr(1));
            if (c == places.end()) {
                currentLocation -= s->data.size();
                symtab[s->id]->offset = currentLocation;
            }
            else {
                symtab[s->id]->offset = c->second;
            }
        }
        //TODO

        // resolve rels
        for(auto i: sections) {
            auto sec = section_map.find(i->name);

            Section * section = sec->second;

            for (auto r: i->rel) {
                
            }
        }
    }


    output(out);
}

void Linker::parseFile(string name) {
    ELFFile * elf = Reader(name).read();
    files.push_back(elf);
}

void Linker::output(string out) {
    FILE * file = fopen(out.c_str(), "wb");
    
    // if (mode == Mode::linkable) {
        Printer(sections,
                symtab,
                file).HumanPrint();
    // }

    fclose(file);
}
