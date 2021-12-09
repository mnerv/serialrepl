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

    //noecho();
    cbreak();  // Breaks out of the program when ^C
    //raw();     // Raw key input
    auto infowin = newwin(3, display.get_width(), 0, 0);
    auto inputwin = newwin(3, display.get_width(), display.get_height() - 4, 0);
    auto outputwin = newwin(display.get_height() - 6, display.get_width(), 2, 0);
    refresh();

    wrefresh(outputwin);
    mvwprintw(infowin, 1, 1, "port: /dev/tty.usbserial-014CA306");
    box(infowin, 0, 0);
    wrefresh(infowin);
    box(inputwin, 0, 0);
    wrefresh(inputwin);
    keypad(inputwin, true);

    bool is_running = true;
    std::string input;
    wmove(inputwin, 1, 1);
    while(is_running) {
        // read user input
        auto c = wgetch(inputwin);
        input += char(c);

        //mvwprintw(outputwin, 1, 1, "%d, %d", KEY_ENTER, c);
        //wrefresh(outputwin);
        if (c == '\n') {
            outputs.push_back(input);
            input = "";

            auto output_size = outputs.size();
            auto start = 0;
            if (int32_t(output_size) > display.get_height() - 6) {
                start = output_size - display.get_height() - 6;
            }
            wclear(outputwin);
            for (int32_t i = start; i < int32_t(outputs.size()); i++) {
                mvwprintw(outputwin, i + 1, 1, "%s", outputs[i].c_str());
                wrefresh(outputwin);
            }

            wclear(inputwin);
            box(inputwin, 0, 0);
            wmove(inputwin, 1, 1);
            wrefresh(inputwin);
        }
    }

    return 0;
}

