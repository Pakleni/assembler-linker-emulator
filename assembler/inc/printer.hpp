#include <stdio.h>
#include <string>

class Printer {
    private:
    Printer(){};

public:
    ~Printer(){}
    Printer(Printer const &) = delete;
    void operator=(Printer const &) = delete;
    static Printer &getInstance()
    {
        static Printer instance;
        return instance;
    }
    void Print();
    void HumanPrint();
};