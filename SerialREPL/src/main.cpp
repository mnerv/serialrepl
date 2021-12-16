/**
 * @file   main.cpp
 * @author Pratchaya Khansomboon (pratchaya.k.git@gmail.com)
 * @brief  Simple Serial REPL program with ncurses
 * @date   2021-12-11
 *
 * @copyright Copyright (c) 2021
 */
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

constexpr std::size_t BUFFER_SIZE = 1024;

std::vector<std::string> history;
std::vector<std::string> outputs;
std::mutex mutex;
WINDOW* infowin  = nullptr;
WINDOW* inputwin = nullptr;
WINDOW* outputwin = nullptr;
char raw_buffer[BUFFER_SIZE];
char eol = '\r';

auto read_data(asio::serial_port& serial) -> void {
    serial.async_read_some(asio::buffer(raw_buffer, BUFFER_SIZE),
            [&](asio::error_code const& ec, std::size_t length) {
                std::scoped_lock<std::mutex> lock(mutex);
                std::string output;
                if (ec) {
                    read_data(serial);
                    return;
                }
                for (int32_t i = 0; i < int32_t(length); i++)
                    output += raw_buffer[i];

                if (outputwin != nullptr) {
                    outputs.push_back(output);

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
                read_data(serial);
            });
}

auto main([[maybe_unused]]int32_t argc, [[maybe_unused]]char const* argv[]) -> int32_t {
    sr::display display;
    if (!has_colors()) {
        std::cerr << "Terminal does not support color\n";
        return 1;
    }

    noecho();
    cbreak();  // Breaks out of the program when ^C
    //raw();     // Raw key input
    infowin = newwin(3, display.get_width(), 0, 0);
    inputwin = newwin(3, display.get_width(), display.get_height() - 4, 0);
    outputwin = newwin(display.get_height() - 6, display.get_width(), 2, 0);
    refresh();

    wrefresh(outputwin);
    mvwprintw(infowin, 1, 1, "port: /dev/tty.usbserial-014CA306  baudrate: 115200");
    wrefresh(outputwin);
    box(infowin, 0, 0);
    wrefresh(infowin);
    box(inputwin, 0, 0);
    wrefresh(inputwin);
    keypad(inputwin, true);

    int32_t history_index = -1;
    bool is_running       = true;
    std::string input{};

    using namespace std::chrono_literals;
    std::string port{argv[1]};

    asio::error_code ec;
    asio::io_context io_context;
    asio::io_service::work idle_work(io_context);
    std::thread thread_context([&]() { io_context.run(); });
    asio::serial_port serial(io_context);

    serial.open(port, ec);
    if (ec) {
        std::cerr << ec.message() << '\n';
        io_context.stop();
        thread_context.join();
        return 1;
    }
    serial.set_option(asio::serial_port_base::baud_rate(115200));
    serial.set_option(asio::serial_port_base::character_size(8));
    serial.set_option(asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one));
    serial.set_option(asio::serial_port_base::parity(asio::serial_port_base::parity::none));
    serial.set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none));

    wclear(outputwin);
    read_data(serial);

    wmove(inputwin, 1, 1);
    while(is_running) {
        wclear(inputwin);
        box(inputwin, 0, 0);
        mvwprintw(inputwin, 1, 1, ">%s", input.c_str());
        wrefresh(inputwin);

        // read user input
        auto c = wgetch(inputwin);

        //mvwprintw(outputwin, 1, 1, "%d", c);
        //wrefresh(outputwin);

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
            if (input == "clear")
                outputs.clear();
            else {
                history.push_back(input);
                outputs.push_back(input);
                serial.write_some(asio::buffer(input.data(), input.size()), ec);
            }
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

    io_context.stop();
    thread_context.join();

    return 0;
}

