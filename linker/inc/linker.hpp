#include <string>
#include <vector>
#include <map>
#include <list>
#include <unordered_set>

#include "../inc/reader.hpp"

class Linker
{
private:
    Linker(){};

public:
    std::vector <ELFFile *> files;
    
    ~Linker();
    Linker(Linker const &) = delete;
    void operator=(Linker const &) = delete;
    static Linker &getInstance()
    {
        static Linker instance;
        return instance;
    }

    enum Mode
    {
        none,
        hex,
        linkable
    };

    void parsePlace(std::string s);
    void setMode(Mode _mode) { mode = _mode; };
    void addSource(std::string source) { sources.push_back(source); }
    void start(std::string output);

    void parseFile(std::string file);
    void output(std::string file);
    void resolve(ELFFile * f);
    void addUnresolved(ELFFile * f);

    void insertIntoMemory(Section * sec, int place);
    void insertIntoMemory(Section * sec);
    void HexPrint(FILE * file);

    std::list<Section *> memory;
    std::vector<SymTabEntry *> symtab;

private:
    std::map <std::string, std::pair<SymTabEntry*, ELFFile*>> resolved;
    std::map<std::string, SymTabEntry *> unresolved;

    std::vector<Section *> sections;
    std::map<std::string, Section *> section_map;

    bool testInsertion (Section * sec,
                        std::_List_iterator<Section *> &i);

    uint16_t ADDR(int entry);

    std::vector<std::string> sources;
    std::map<std::string, int> places;
    Mode mode = none;
};