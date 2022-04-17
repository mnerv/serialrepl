#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <cstdint>
#include <chrono>
#include <mutex>

#define ASIO_STANDALONE
#include "asio.hpp"

auto main([[maybe_unused]]std::int32_t argc, [[maybe_unused]]char const* argv[]) -> std::int32_t {
    std::cout << "Hello, Serial REPL!\n";
    return 0;
}

