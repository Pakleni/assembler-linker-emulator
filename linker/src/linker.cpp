#include "../inc/linker.hpp"
#include "../../assembler/inc/printer.hpp"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

void Linker::insertIntoMemory(Section *sec, int place)
{

    if (memory.size() == 0)
    {
        symtab[sec->id]->offset = place;
        memory.push_back(sec);
        return;
    }

    for (auto i = memory.begin(); i != memory.end(); ++i)
    {
        if (symtab[(*i)->id]->offset > place)
        {
            Section *nx = (*i);
            if (i != memory.begin())
            {
                Section *pr = (*(--i));

                if ((symtab[pr->id]->offset + pr->data.size() < place) &&
                    (symtab[nx->id]->offset >= place + sec->data.size()))
                {

                    symtab[sec->id]->offset = place;
                    memory.insert(++i, sec);
                    return;
                }
            }
            else
            {
                if (symtab[nx->id]->offset >= place + sec->data.size())
                {
                    symtab[sec->id]->offset = place;
                    memory.insert(i, sec);
                    return;
                }
            }

            cout << "Some memory overlaps" << endl;
            exit(1);
        }
    }

    Section *pr = *memory.rbegin();
    if (symtab[pr->id]->offset + pr->data.size() < place)
    {
        symtab[sec->id]->offset = place;
        memory.push_back(sec);
        return;
    }
    else
    {
        cout << "Some memory overlaps" << endl;
        exit(1);
    }
}

bool Linker::testInsertion(Section *sec, std::_List_iterator<Section *> &i)
{
    if (i != --memory.end())
    {
        Section *pr = *i;
        Section *nx = *(++i);

        uint16_t place = symtab[nx->id]->offset - sec->data.size();

        if (symtab[pr->id]->offset + pr->data.size() <= place)
        {
            symtab[sec->id]->offset = place;
            i = memory.insert(i, sec);
            return true;
        }
        else --i;
    }
    else
    {
        Section *pr = *(i);

        uint16_t place = 0xFF00 - sec->data.size();

        if (symtab[pr->id]->offset + pr->data.size() <= place)
        {
            symtab[sec->id]->offset = place;
            i = memory.insert(memory.end(), sec);
            return true;
        }
    }

    return false;
}

void Linker::insertIntoMemory(Section *sec)
{

    static auto last = memory.end();

    if (memory.size() == 0)
    {
        symtab[sec->id]->offset = 0xFF00 - sec->data.size();
        memory.push_back(sec);
        last = --memory.end();
        return;
    }

    std::_List_iterator<Section *> temp = --last;

    for (; last != --memory.begin(); --last)
    {
        if (testInsertion(sec, last))
        {
            return;
        }
    }

    for (last = --memory.end(); last != --memory.begin(); --last)
    {
        if (testInsertion(sec, last))
        {
            return;
        }
    }

    cout << "Can't place all sections into memory" << endl;
    exit(1);
}

void Linker::parsePlace(string s)
{
    auto i = s.find("=");

    if (i == string::npos)
    {
        return;
    }

    string s_new = s.substr(i + 1);

    i = s_new.find("@");

    if (i == string::npos)
    {
        cout << "Error in -place: " << s << endl;
        return;
    }

    string s1 = s_new.substr(0, i);
    string s2 = s_new.substr(i + 1);

    int x;
    std::stringstream ss;
    ss << std::hex << s2;
    ss >> x;

    places.insert({s1, x});
}

Linker::~Linker()
{
    while (!files.empty())
    {
        delete files.back();
        files.pop_back();
    }
}

void Linker::resolve(ELFFile *f)
{
    for (auto i : f->symtab)
    {
        if (!i->isSection && !i->isLocal && i->section != EXTERNAL_SECTION)
        {
            if (resolved.find(i->label) != resolved.end())
            {
                cout << "Symbol " << i->label << " defined more than once." << endl;
                exit(1);
            }

            SymTabEntry *add = new SymTabEntry(*i);
            add->id = symtab.size();

            symtab.push_back(add);
            resolved.insert({i->label, {add, f}});

            auto pos = unresolved.find(i->label);
            if (pos != unresolved.end())
            {
                delete pos->second;
                unresolved.erase(pos);
            }
        }
    }
}

void Linker::addUnresolved(ELFFile *f)
{
    for (auto i : f->symtab)
    {
        if (i->section == EXTERNAL_SECTION)
        {
            unresolved.insert({i->label, new SymTabEntry(*i)});
        }
    }
}

void Linker::start(string out)
{
    if (mode == none)
        return;

    for (auto i : sources)
    {
        parseFile(i);
    }

    for (ELFFile *f : files)
    {
        //f-ovim defined, razresi unres slevo
        resolve(f);
        addUnresolved(f);
    }
    if (unresolved.size() > 0)
    {
        if (mode == Mode::hex)
        {
            cout << "Theres still unresolved symbols!" << endl;
            exit(1);
        }

        for (auto i : unresolved)
        {
            auto sym = i.second;
            sym->id = symtab.size();
            symtab.push_back(sym);
            resolved.insert({sym->label, {sym, nullptr}});
        }
    };

    //add all sections to sections and section_map and symtab
    for (ELFFile *f : files)
    {
        for (auto i : f->sections)
        {
            auto sec = section_map.find(i->name);

            Section *section;

            if (sec == section_map.end())
            {
                //create entry for section
                SymTabEntry *sect = new SymTabEntry(i->name, sections.size(), 0);
                sect->isSection = true;
                sect->id = symtab.size();
                symtab.push_back(sect);
                resolved.insert({i->name, {sect, nullptr}});
                //add section to list
                section = new Section(i->name, sect->id);
                sections.push_back(section);
                section_map.insert({section->name, section});
            }
            else
            {
                section = sec->second;
            }

            //offset of my section
            f->symtab[i->id]->offset = section->data.size();

            for (auto d : i->data)
            {
                section->data.push_back(d);
            }
        }
    }

    //generate rel entries
    for (ELFFile *f : files)
    {
        for (auto i : f->sections)
        {
            auto sec = section_map.find(i->name);

            Section *section = sec->second;

            int off = f->symtab[i->id]->offset;

            for (auto r : i->rel)
            {
                string name = f->symtab[r->entry]->label;
                int id = resolved.find(name)->second.first->id;
                RelEntry *rel = new RelEntry(r->offset + off, r->relType, id);

                section->rel.push_back(rel);
            }
        }
    }

    //resolve multiple same sections
    for (auto i : resolved)
    {

        auto sym = i.second.first;

        if (sym->section != ABSOLUTE_SECTION)
        {
            auto f = i.second.second;
            if (f == nullptr)
                continue; //sekcija je ili undef
            auto old_section = f->sections[sym->section];
            //append offset
            sym->offset += f->symtab[old_section->id]->offset;
            //get id of new section
            sym->section = symtab[section_map.find(old_section->name)->second->id]->section;
        }
    }

    if (mode == Mode::hex)
    {
        //locations of sections
        list<Section *> not_in_memory;

        for (auto s : sections)
        {
            auto c = places.find(s->name.substr(1));
            if (c == places.end())
            {
                not_in_memory.push_back(s);
            }
            else
            {
                insertIntoMemory(s, c->second);
            }
        }

        while (!not_in_memory.empty())
        {
            insertIntoMemory(not_in_memory.front());
            not_in_memory.pop_front();
        }

        // resolve rels
        for (auto i : sections)
        {
            auto sec = section_map.find(i->name);

            Section *section = sec->second;

            for (auto r : i->rel)
            {
                uint16_t addend = i->data[r->offset] + (i->data[r->offset + 1] << 8);

                uint16_t res;

                if (r->relType == RelEntry::R_PC16)
                {
                    uint16_t adr = symtab[i->id]->offset + r->offset;

                    res = ADDR(r->entry) + addend - adr;
                }
                else
                {
                    res = ADDR(r->entry) + addend;
                }
                i->data[r->offset] = res & 0x00ff;
                i->data[r->offset + 1] = (res & 0xff00) >> 8;
            }
        }
    }

    output(out);
}

uint16_t Linker::ADDR(int entry)
{
    auto sym = symtab[entry];

    if (sym->isSection)
    {
        return sym->offset;
    }
    else
    {
        return symtab[sections[sym->section]->id]->offset + sym->offset;
    }
}

void Linker::parseFile(string name)
{
    ELFFile *elf = Reader(name).read();
    files.push_back(elf);
}

void Linker::output(string out)
{

    FILE * file = fopen(out.c_str(), "wb");

    if (mode == Mode::linkable)
    {
        Printer(sections,
                symtab,
                file)
            .HumanPrint();

        file = freopen((out+".bin").c_str(), "wb", file);
        Printer(sections,
                symtab,
                file)
            .Print();
    }
    else
    {
        HexPrint(file);
    }

    fclose(file);
}

inline void prefill (FILE * file, uint16_t& temp) {
    if ((temp % 8) == 0) return;

    fprintf(file, "%.4X:", temp - temp%8);
    for (int i = 0;i < (temp % 8);++i) {
        fprintf(file, " 00");
    }
}

inline void postfill (FILE * file, uint16_t& temp) {
    if ((temp % 8) == 0) return;

    for (;(temp % 8)!=0;++temp) {
        fprintf(file, " 00");
    }
    fprintf(file, "\n");
}

void Linker::HexPrint(FILE * file)
{

    uint16_t temp = 0; 
    for (auto i = memory.begin(); i != memory.end(); ++i)
    {
        
        uint16_t curr = symtab[(*i)->id]->offset;

        if (curr != temp) {
            postfill(file, temp);
            temp = curr;
            prefill(file, curr);
        }

        for (int j = 0; j < (*i)->data.size(); ++j) {
            if ((temp % 8) == 0) {
                fprintf(file, "%.4X:", temp);
            }

            fprintf(file, " %.2X", (*i)->data[j]);

            if ((temp % 8) == 7) {
                fprintf(file, "\n");
            }
            ++temp;
        }
    }
    postfill(file, temp);
}