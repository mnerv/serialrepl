#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <cstdint>
#include <chrono>
#include <mutex>

#define ASIO_STANDALONE
#include "asio.hpp"

constexpr auto BUFFER_SIZE = 256;

constexpr auto USAGE = R"(
usage: HelloSerial [commands] [port]
)";

std::mutex mutex;
char raw_buffer[BUFFER_SIZE];
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
                    std::cout << raw_buffer[i];
                std::cout << '\n';
                read_data(serial);
            });
}

auto main([[maybe_unused]]int32_t argc, [[maybe_unused]]char const* argv[]) -> int32_t {
    if (argc < 3) {
        std::cout << USAGE;
        return 1;
    };
    asio::error_code ec;

    using namespace std::chrono_literals;
    std::string commands_file{argv[1]};
    std::string port{argv[2]};

    asio::io_service io_service;
    // Make the context idle so we don't exit the program
    asio::io_context::work idle_work(io_service);
    // Start in another thread
    std::thread thread_context = std::thread([&]() { io_service.run(); });

    asio::serial_port serial(io_service);

    std::cout << "Opening Serial Port... ";
    serial.open(port, ec);
    if (ec) {
        std::cerr << ec.message() << '\n';
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
        //serial.write_some(asio::buffer(input.data(), input.size()), ec);
        if (input.size() == 0)
            continue;
        serial.write_some(asio::buffer(input + "\r"), ec);
        std::this_thread::sleep_for(125ms);
    }

    thread_context.join();

    return 0;
}

