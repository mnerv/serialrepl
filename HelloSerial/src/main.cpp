#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <cstdint>
#include <chrono>
#include <mutex>

#define ASIO_STANDALONE
#include "asio.hpp"

constexpr auto BUFFER_SIZE = 1024;
constexpr auto USAGE =
"usage: HelloSerial [commands] [port]";

std::mutex mutex;
char raw_buffer[BUFFER_SIZE];
char print_buffer[BUFFER_SIZE + 1];
char eol = '\r';

auto read_data(asio::serial_port& serial) -> void {
    serial.async_read_some(asio::buffer(raw_buffer, BUFFER_SIZE),
            [&](asio::error_code const& ec, std::size_t length) {
                std::scoped_lock<std::mutex> lock(mutex);
                if (ec) {
                    read_data(serial);
                    return;
                }
                for (int32_t i = 0; i < int32_t(length); i++)
                    print_buffer[i] = raw_buffer[i];
                print_buffer[length] = '\0';
                std::cout << print_buffer;
                std::cout << '\n';
                read_data(serial);
            });
}

auto main(int32_t argc, char const* argv[]) -> int32_t {
    if (argc < 3) {
        std::cout << USAGE << '\n';
        return 1;
    };
    asio::error_code ec;

    using namespace std::chrono_literals;
    std::string commands_file{argv[1]};
    std::string port{argv[2]};

    asio::io_service io_service;
    asio::io_service::work idle_work(io_service);
    std::thread thread_context([&]() { io_service.run(); });
    asio::serial_port serial(io_service);

    std::cout << "Opening Serial Port... ";
    serial.open(port, ec);
    if (ec) {
        std::cerr << ec.message() << '\n';
        io_service.stop();
        thread_context.join();
        return 1;
    }
    serial.set_option(asio::serial_port_base::baud_rate(115200));
    serial.set_option(asio::serial_port_base::character_size(8));
    serial.set_option(asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one));
    serial.set_option(asio::serial_port_base::parity(asio::serial_port_base::parity::none));
    serial.set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none));
    std::cout << "Opened!\n";

    read_data(serial);
    while(true) {
        std::string input;
        std::cout << "> ";
        std::cin >> input;
        if (input.size() == 0)
            continue;
        input += eol;
        serial.write_some(asio::buffer(input.data(), input.size()), ec);
        std::this_thread::sleep_for(125ms);
    }

    io_service.stop();
    thread_context.join();

    return 0;
}

