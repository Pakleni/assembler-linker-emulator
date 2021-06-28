#include <string>
#include <thread>
#include <mutex>


class Emulator {
    Emulator(){};
public:
    ~Emulator();
    Emulator(Emulator const &) = delete;
    void operator=(Emulator const &) = delete;

    uint8_t  memory[1 << 16];
    uint16_t  regs[9]; // 6-SP 7-PC 8-PSW
    uint8_t* ivt = memory;

    static Emulator &getInstance()
    {
        static Emulator instance;
        return instance;
    }

    void push(uint8_t reg);
    void pop(uint8_t reg);

    void start(std::string in);
    void readHex(std::string in);

    bool parseInstruction();

    void interrupt(uint8_t RD);
    void iret();

    void call(uint16_t op);
    void ret();

    void jump(uint16_t op);
    void jeq(uint16_t op);
    void jne(uint16_t op);
    void jgt(uint16_t op);

    void xchg(uint8_t RD);

    void add(uint8_t RD);
    void sub(uint8_t RD);
    void mul(uint8_t RD);
    void div(uint8_t RD);
    void cmp(uint8_t RD);

    void i_not(uint8_t RD);
    void i_and(uint8_t RD);
    void i_or(uint8_t RD);
    void i_xor(uint8_t RD);
    void i_test(uint8_t RD);

    void shl(uint8_t RD);
    void shr(uint8_t RD);

    void load(uint8_t RD, uint16_t operand);

    void store(uint8_t RD, uint8_t am, uint8_t up);
    void store(uint8_t RD, uint8_t am, uint8_t up, uint16_t data);

    void error();
    void timerInterrupt();
    void terminalInterrupt();
    void timerFunction();
    void terminalFunction(char c);

    std::mutex memory_mutex;

    volatile bool timer = false;
    volatile bool terminal = false;
    volatile bool isRunning = false;
};