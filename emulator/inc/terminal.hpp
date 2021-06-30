#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

class Terminal
{
    Terminal(){};

public:
    ~Terminal(){ terminal_done(); };
    Terminal(Terminal const &) = delete;
    void operator=(Terminal const &) = delete;

    static Terminal &getInstance()
    {
        static Terminal instance;
        return instance;
    }

    int descriptor = -1;
    struct termios backup;
    struct termios settings;

    int terminal_init()
    {
        if (descriptor != -1)
            return EXIT_SUCCESS;

        if (isatty(STDERR_FILENO))
            descriptor = STDERR_FILENO;
        else if (isatty(STDIN_FILENO))
            descriptor = STDIN_FILENO;
        else if (isatty(STDOUT_FILENO))
            descriptor = STDOUT_FILENO;
        else
            return EXIT_FAILURE;

        //Terminal setting
        if (tcgetattr(descriptor, &backup) ||
            tcgetattr(descriptor, &settings))
            return EXIT_FAILURE;

        //Disable buffering for terminal streams.
        if (isatty(STDIN_FILENO))
            setvbuf(stdin, NULL, _IONBF, 0);
        if (isatty(STDOUT_FILENO))
            setvbuf(stdout, NULL, _IONBF, 0);
        if (isatty(STDERR_FILENO))
            setvbuf(stderr, NULL, _IONBF, 0);

        //dont translate \n
        settings.c_iflag &= ~(INLCR | IGNCR | ICRNL);

        //8-bit chars
        settings.c_cflag &= ~CSIZE;
        settings.c_cflag |= CS8;
        settings.c_cflag |= CREAD;

        //Enable default signals
        settings.c_lflag |= ISIG;

        ///Disable line buffering
        settings.c_lflag &= ~ICANON;

        //Disable echoing
        settings.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

        //Disable input processing.
        settings.c_lflag &= ~IEXTEN;

        //read() will block until at least one byte is available.
        settings.c_cc[VTIME] = 0;
        settings.c_cc[VMIN] = 1;

        tcsetattr(descriptor, TCSANOW, &settings);

        return EXIT_SUCCESS;
    }

    static void terminal_done()
    {
        if (Terminal::getInstance().descriptor != -1)
            tcsetattr(Terminal::getInstance().descriptor,
                      TCSANOW,
                      &Terminal::getInstance().backup);
    }
};
