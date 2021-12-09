#include "display.hpp"

#include <locale>
#include <iostream>

namespace sr {

display::display() {
    setlocale(LC_ALL, "");
    initscr();

    keypad(stdscr, true);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);
    std::cerr << "\033[?1003h\n";

    m_width  = getmaxx(stdscr);
    m_height = getmaxy(stdscr);
}

display::~display() {
    std::cerr << "\033[?1003l\n";
    endwin();
}

auto display::get_width() -> int32_t {
    m_width = getmaxx(stdscr);
    return m_width;
}
auto display::get_height() -> int32_t {
    m_height = getmaxy(stdscr);
    return m_height;
}

}
