#include <stdio.h>
#include <cstdint>

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
    ELFSTEntry() {}

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
    ELFHeader() {}

    void write(FILE * file) { fwrite(this, sizeof(*this), 1, file); }
};

struct ELFRelEntry
{
    uint32_t offset;
    uint32_t info;

    ELFRelEntry(uint32_t _of, uint32_t _in) : offset(_of), info(_in) {}
    ELFRelEntry() {}
    void write(FILE * file) { fwrite(this, sizeof(*this), 1, file); }
};
