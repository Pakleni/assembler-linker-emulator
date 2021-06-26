#include <string>
#include <vector>
#include <map>
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

private:
    std::vector<std::string> sources;
    std::map<std::string, int> places;
    Mode mode = none;
};