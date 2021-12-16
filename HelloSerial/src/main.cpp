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

std::mutex mutex;
char raw_buffer[BUFFER_SIZE];
char print_buffer[BUFFER_SIZE + 1];
char eol = '\r';

auto print_usage([[maybe_unused]]int32_t argc, char const* argv[]) -> void {
    std::cout << "usage: " << argv[0] << " <serial_port> <commands_file>\n\n";
    std::cout << "  serial_port:    serial port  (required)\n";
    std::cout << "  command_file:   command file (optional)\n";
}

auto read_data(asio::serial_port& serial) -> void {
    serial.async_read_some(asio::buffer(raw_buffer, BUFFER_SIZE),
            [&](asio::error_code const& ec, std::size_t length) {
                //std::scoped_lock<std::mutex> lock(mutex);
                if (ec) {
                    read_data(serial);
                    return;
                }
                for (int32_t i = 0; i < int32_t(length); i++)
                    print_buffer[i] = raw_buffer[i];
                print_buffer[length] = '\0';
                std::cout << print_buffer;
                read_data(serial);
            });
}

auto main(int32_t argc, char const* argv[]) -> int32_t {
    if (argc < 2) {
        print_usage(argc, argv);
        return 1;
    };
    asio::error_code ec;

    using namespace std::chrono_literals;
    std::string port{argv[1]};
    //std::string commands_file{argv[2]};

    asio::io_context io_context;
    asio::io_service::work idle_work(io_context);
    std::thread thread_context([&]() { io_context.run(); });
    asio::serial_port serial(io_context);

    std::cout << "opening serial port...\n";
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
    std::cout << "opened port: " << port << "\n";

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

    io_context.stop();
    thread_context.join();

    return 0;
}

