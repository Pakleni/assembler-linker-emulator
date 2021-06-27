#include <string>

class Emulator {
    Emulator(){};
public:
    ~Emulator();
    Emulator(Emulator const &) = delete;
    void operator=(Emulator const &) = delete;
    static Emulator &getInstance()
    {
        static Emulator instance;
        return instance;
    }

    void start(std::string in);
};