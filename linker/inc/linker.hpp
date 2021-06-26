#include <string>
#include <vector>
#include <map>
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


private:
    std::map <std::string, std::pair<SymTabEntry*, ELFFile*>> resolved;
    std::map<std::string, SymTabEntry *> unresolved;

    std::vector<SymTabEntry *> symtab;
    std::vector<Section *> sections;
    std::map<std::string, Section *> section_map;



    std::vector<std::string> sources;
    std::map<std::string, int> places;
    Mode mode = none;
};