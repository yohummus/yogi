#include "console.hpp"

#include <iostream>
#include <string>
using namespace std::string_literals;

#ifdef _WIN32
#   include <Windows.h>
#else
#   include <unistd.h>
#endif


namespace yogi {
namespace internal {
namespace {

#ifdef _WIN32
auto win32OriginalColors = []() {
    HANDLE hdl = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hdl, &csbi);
    return csbi.wAttributes;
}();
#endif
} // anonymous namespace

void set_console_title(const std::string& title)
{
#ifdef _WIN32
    SetConsoleTitleA(title.c_str());
#else
    if (isatty(STDOUT_FILENO)) {
        auto seq = "\033]0;"s + title + "\007";
        std::cout << seq << std::flush;
    }
#endif
}

void set_console_colour(foreground_colour colour)
{
#ifdef _WIN32
    HANDLE hdl = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hdl, &csbi);
    SetConsoleTextAttribute(hdl, (csbi.wAttributes & 0xFFF0) | static_cast<WORD>(colour));
#else
    if (!isatty(STDOUT_FILENO)) {
        return;
    }

    const char* seq;
    switch (colour) {
    case foreground_colour::BLACK:         seq = "\033[22;30m"; break;
    case foreground_colour::BLUE:          seq = "\033[22;34m"; break;
    case foreground_colour::GREEN:         seq = "\033[22;32m"; break;
    case foreground_colour::CYAN:          seq = "\033[22;36m"; break;
    case foreground_colour::RED:           seq = "\033[22;31m"; break;
    case foreground_colour::MAGENTA:       seq = "\033[22;35m"; break;
    case foreground_colour::BROWN:         seq = "\033[22;33m"; break;
    case foreground_colour::GREY:          seq = "\033[22;37m"; break;
    case foreground_colour::DARK_GREY:     seq = "\033[01;30m"; break;
    case foreground_colour::LIGHT_BLUE:    seq = "\033[01;34m"; break;
    case foreground_colour::LIGHT_GREEN:   seq = "\033[01;32m"; break;
    case foreground_colour::LIGHT_CYAN:    seq = "\033[01;36m"; break;
    case foreground_colour::LIGHT_RED:     seq = "\033[01;31m"; break;
    case foreground_colour::LIGHT_MAGENTA: seq = "\033[01;35m"; break;
    case foreground_colour::YELLOW:        seq = "\033[01;33m"; break;
    case foreground_colour::WHITE:         seq = "\033[01;37m"; break;
    default:                               seq = "";            break;
    }

    std::cout << seq;
#endif
}

void set_console_colour(background_colour colour)
{
#ifdef _WIN32
    HANDLE hdl = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hdl, &csbi);
    SetConsoleTextAttribute(hdl, (csbi.wAttributes & 0xFF0F) | (static_cast<WORD>(colour) << 4));
#else
    if (!isatty(STDOUT_FILENO)) {
        return;
    }

    const char* seq;
    switch (colour) {
    case background_colour::BLACK:   seq = "\033[40m"; break;
    case background_colour::RED:     seq = "\033[41m"; break;
    case background_colour::GREEN:   seq = "\033[42m"; break;
    case background_colour::YELLOW:  seq = "\033[43m"; break;
    case background_colour::BLUE:    seq = "\033[44m"; break;
    case background_colour::MAGENTA: seq = "\033[45m"; break;
    case background_colour::CYAN:    seq = "\033[46m"; break;
    case background_colour::WHITE:   seq = "\033[47m"; break;
    default:                         seq = "";         break;
    }

    std::cout << seq;
#endif
}

void reset_console_colours()
{
#ifdef _WIN32
    HANDLE hdl = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hdl, win32OriginalColors);
#else
    if (!isatty(STDOUT_FILENO)) {
        return;
    }

    std::cout << "\033[0m";
#endif
}

} // namespace internal
} // namespace yogi
