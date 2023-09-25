#pragma once


#include "../ucanopen_def.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>


namespace ucanopen {

class Tester {
private:
    std::chrono::time_point<std::chrono::steady_clock> _init_timepoint;
public:
    Tester() {
        _init_timepoint = std::chrono::steady_clock::now();
    }

    auto timestamp() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _init_timepoint).count();
    }

    std::array<uint8_t, 8> make_tpdo1() {
        std::cout << "[ucanopen] TPDO1"
                  << " | timestamp: " << std::dec << std::setw(7) << timestamp() << "ms"
                  << " | thread: " << std::this_thread::get_id()
                  << std::endl;
        return {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xFF};
    }

    std::array<uint8_t, 8> make_tpdo2() {
        std::cout << "[ucanopen] TPDO2"
                  << " | timestamp: " << std::dec << std::setw(7) << timestamp() << "ms"
                  << " | thread: " << std::this_thread::get_id()
                  << std::endl;
        return {0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0xFF};
    }

    std::array<uint8_t, 8> make_tpdo3() {
        std::cout << "[ucanopen] TPDO3"
                  << " | timestamp: " << std::dec << std::setw(7) << timestamp() << "ms"
                  << " | thread: " << std::this_thread::get_id()
                  << std::endl;
        return {0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0xFF};
    }

    std::array<uint8_t, 8> make_tpdo4() {
        std::cout << "[ucanopen] TPDO4"
                  << " | timestamp: " << std::dec << std::setw(7) << timestamp() << "ms"
                  << " | thread: " << std::this_thread::get_id()
                  << std::endl;
        return {0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0xFF};
    }

    void process_rpdo1(std::array<uint8_t, 8> data) {
        std::cout << "[ucanopen] RPDO1"
                  << " | timestamp: " << std::dec << std::setw(7) << timestamp() << "ms"
                  << " | thread: " << std::this_thread::get_id()
                  << " | data: ";

        for (uint8_t val : data) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << int(val) << " ";
        }
        std::cout << std::endl;
    }

    void process_rpdo2(std::array<uint8_t, 8> data) {
        std::cout << "[ucanopen] RPDO2"
                  << " | timestamp: " << std::dec << std::setw(7) << timestamp() << "ms"
                  << " | thread: " << std::this_thread::get_id()
                  << " | data: ";

        for (uint8_t val : data) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << int(val) << " ";
        }
        std::cout << std::endl;
    }

    void process_rpdo3(std::array<uint8_t, 8> data) {
        std::cout << "[ucanopen] RPDO3"
                  << " | timestamp: " << std::dec << std::setw(7) << timestamp() << "ms"
                  << " | thread: " << std::this_thread::get_id()
                  << " | data: ";

        for (uint8_t val : data) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << int(val) << " ";
        }
        std::cout << std::endl;
    }

    void process_rpdo4(std::array<uint8_t, 8> data) {
        std::cout << "[ucanopen] RPDO4"
                  << " | timestamp: " << std::dec << std::setw(7) << timestamp() << "ms"
                  << " | thread: " << std::this_thread::get_id()
                  << " | data: ";

        for (uint8_t val : data) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << int(val) << " ";
        }
        std::cout << std::endl;
    }
};

} // namespace ucanopen

