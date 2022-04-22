#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <cstdint>
#include <chrono>
#include <mutex>
#include <cstdlib>

#define ASIO_STANDALONE
#include "asio.hpp"

constexpr std::size_t BUFFER_SIZE = 512;
static char raw_buffer[BUFFER_SIZE]{};
static auto is_running = true;

namespace sr {
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

auto arg_device(std::vector<std::string> const& args) -> std::string {
    if (args.size() < 2) return "";
    return args[1];
}
}

auto main(std::int32_t argc, char const* argv[]) -> std::int32_t {
    std::signal(SIGINT, [](std::int32_t) { is_running = false; });

    // create arguments vector
    std::vector<std::string> args{argv, argv + argc};

    std::string device{sr::arg_device(args)};
    std::uint32_t baud_rate = 115200;

    asio::io_context io_context{};
    asio::io_service::work idle_work{io_context};
    std::thread thread_context{[&] { io_context.run(); }};

    // open serial port device
    asio::error_code err_code;
    asio::serial_port serial{io_context};
    serial.open(device, err_code);
    if (err_code) {
        std::cerr << err_code.message() << "\n";
        io_context.stop();
        thread_context.join();
        return 1;
    }
    serial.set_option(asio::serial_port_base::baud_rate{baud_rate});
    serial.set_option(asio::serial_port_base::character_size{8});
    serial.set_option(asio::serial_port_base::stop_bits{asio::serial_port_base::stop_bits::one});
    serial.set_option(asio::serial_port_base::parity{asio::serial_port_base::parity::none});
    serial.set_option(asio::serial_port_base::flow_control{asio::serial_port_base::flow_control::none});

    sr::async_read_data(serial);

    using namespace std::chrono_literals;
    while (is_running) {
        std::this_thread::sleep_for(125ms);
    }

    io_context.stop();
    thread_context.join();
    return 0;
}

