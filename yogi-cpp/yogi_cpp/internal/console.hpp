#ifndef YOGICPP_INTERNAL_CONSOLE_HPP
#define YOGICPP_INTERNAL_CONSOLE_HPP

#include <ostream>
#include <string>


namespace yogi {
namespace internal {

enum class foreground_colour {
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    GREY,
    DARK_GREY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    YELLOW,
    WHITE
};

enum class background_colour {
    BLACK   = static_cast<int>(foreground_colour::BLACK),
    RED     = static_cast<int>(foreground_colour::RED),
    GREEN   = static_cast<int>(foreground_colour::GREEN),
    YELLOW  = static_cast<int>(foreground_colour::YELLOW),
    BLUE    = static_cast<int>(foreground_colour::BLUE),
    MAGENTA = static_cast<int>(foreground_colour::MAGENTA),
    CYAN    = static_cast<int>(foreground_colour::CYAN),
    WHITE   = static_cast<int>(foreground_colour::WHITE)
};

void set_console_title(const std::string& title);
void set_console_colour(foreground_colour colour);
void set_console_colour(background_colour colour);
void reset_console_colours();

} // namespace internal
} // namespace yogi

#endif // YOGICPP_INTERNAL_CONSOLE_HPP
