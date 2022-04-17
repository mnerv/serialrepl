#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <cstdint>
#include <chrono>
#include <mutex>

#define ASIO_STANDALONE
#include "asio.hpp"

constexpr std::size_t BUFFER_SIZE = 512;
static char raw_buffer[BUFFER_SIZE];

auto async_read_data(asio::serial_port& serial) -> void {
    serial.async_read_some(asio::buffer(raw_buffer, BUFFER_SIZE),
    [&](asio::error_code const& ec, std::size_t length){
        if (ec) return async_read_data(serial);
        for (std::size_t i = 0; i < length; i++) {
            std::cout << raw_buffer[i];
        }
        async_read_data(serial);
    });
}

auto main([[maybe_unused]]std::int32_t argc, [[maybe_unused]]char const* argv[]) -> std::int32_t {
    std::string port_name{"/dev/tty.usbserial-01438340"};
    std::uint32_t baud_rate = 115200;

    asio::io_context io{};
    asio::io_service::work idle_work{io};
    std::thread thread_context{[&] { io.run(); }};

    // start serial port
    asio::error_code err_code;
    asio::serial_port serial{io};
    serial.open(port_name, err_code);
    if (err_code) {
        std::cerr << err_code.message() << "\n";
        io.stop();
        thread_context.join();
        return 1;
    }
    serial.set_option(asio::serial_port_base::baud_rate{baud_rate});
    serial.set_option(asio::serial_port_base::character_size{8});
    serial.set_option(asio::serial_port_base::stop_bits{asio::serial_port_base::stop_bits::one});
    serial.set_option(asio::serial_port_base::parity{asio::serial_port_base::parity::none});
    serial.set_option(asio::serial_port_base::flow_control{asio::serial_port_base::flow_control::none});

    async_read_data(serial);

    using namespace std::chrono_literals;
    while (true) {
        std::this_thread::sleep_for(125ms);
    }

    io.stop();
    thread_context.join();

    return 0;
}

