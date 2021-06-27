#pragma once
#include <string>
#include <vector>

#define ABSOLUTE_SECTION -1
#define EXTERNAL_SECTION -2

class SymTabEntry
{
private:
    static int idc;

public:
    int id = idc++;

    std::string label;

    int section;
    uint16_t offset;

    bool isLocal = true;
    int string_table_i = 0;

    bool isSection = false;

    SymTabEntry(std::string _label, int _sec, int _off) : label(_label), section(_sec), offset(_off) {}
};

class RelEntry
{
public:
    enum RelTypes
    {
        R_16,
        R_PC16
    };
    int offset;
    RelTypes relType;

    int entry;

    RelEntry(int _offset, RelTypes _relType, int _entry) : relType(_relType),
                                                           entry(_entry)
    {
        offset = _offset;
    }
};

class Section
{
public:
    std::string name;
    std::vector<RelEntry *> rel;
    std::vector<uint8_t> data;

    int string_table_i = 0;
    int string_table_reli = 0;
    int offset = -1;
    int rel_offset = -1;

    //symbol id
    int id;

    Section(std::string _name, int _id) : name(_name), id(_id) {}
    ~Section()
    {
        while (!rel.empty())
        {
            delete rel.back();
            rel.pop_back();
        }
    }
};