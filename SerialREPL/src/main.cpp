#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

#define ASIO_STANDALONE
#include "asio.hpp"

#include "display.hpp"

std::vector<std::string> history;
std::vector<std::string> outputs;

auto main([[maybe_unused]]int32_t argc, [[maybe_unused]]char const* argv[]) -> int32_t {
    sr::display display;
    if (!has_colors()) {
        std::cerr << "Terminal does not support color\n";
        return 1;
    }

    noecho();
    cbreak();  // Breaks out of the program when ^C
    //raw();     // Raw key input
    auto infowin = newwin(3, display.get_width(), 0, 0);
    auto inputwin = newwin(3, display.get_width(), display.get_height() - 4, 0);
    auto outputwin = newwin(display.get_height() - 6, display.get_width(), 2, 0);
    refresh();

    wrefresh(outputwin);
    mvwprintw(infowin, 1, 1, "port: /dev/tty.usbserial-014CA306  baudrate: 115200");
    box(infowin, 0, 0);
    wrefresh(infowin);
    box(inputwin, 0, 0);
    wrefresh(inputwin);
    keypad(inputwin, true);

    [[maybe_unused]]int32_t history_index = -1;
    bool is_running       = true;
    std::string input{};

    wmove(inputwin, 1, 1);
    while(is_running) {
        wclear(inputwin);
        box(inputwin, 0, 0);
        mvwprintw(inputwin, 1, 1, ">%s", input.c_str());
        wrefresh(inputwin);

        // read user input
        auto c = wgetch(inputwin);

        mvwprintw(outputwin, 1, 1, "%d", c);
        wrefresh(outputwin);

        if (c == 127 || c == 263) {
            input.pop_back();
        } else if (c == 259) { // up
            if (history_index < int32_t(history.size()) - 1)
                history_index++;
        } else if (c == 258) { // down
            if (history_index > -1)
                history_index--;
        } else if (c != '\n' && c >= 32 && c <= 127) {
            history_index = -1;
            input += char(c);
        }

        if (history_index != -1)
            input = history[history.size() - history_index - 1];

        if (c == '\n') {
            if (input == "exit")
                is_running = false;

            history.push_back(input);
            outputs.push_back(input);
            input = "";

            auto output_size = outputs.size();
            wclear(outputwin);
            auto out_height = getmaxy(outputwin);
            int32_t row = 0;
            for (int32_t i = int32_t(output_size); i > -1; i--) {
                mvwprintw(outputwin, out_height - row, 1, "%s", outputs[i].c_str());
                row++;
            }
            wrefresh(outputwin);

            box(infowin, 0, 0);
            wrefresh(infowin);
        }
    }

    return 0;
}

