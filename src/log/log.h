#pragma once


#include <deque>
#include <string>
#include <mutex>
#include <cstring>
#include <iostream>
#include <syncstream>
#include <vector>


extern "C" bool log_get_message(char* buf, int len);


enum class LogPrefix {
    none,
    align,
    ok,
    failed
};


class Log final {
    friend bool log_get_message(char* buf, int len);
private:
    static inline std::vector _message_prefixes = {
        "",
        "         ",
        "[  OK  ] ",
        "[FAILED] "
    };

    static inline std::stringstream _stream;
    std::stringstream _tmp_stream;
public:
    Log() = default;
    Log(const Log& other) = delete;
    Log& operator=(const Log& other) = delete;

    ~Log() {
        _flush(LogPrefix::none);
    }

    template <typename T>
    Log& operator<<(const T& msg) {
        _tmp_stream << msg;
        //std::cout << msg;
        //_stream << msg;
        return *this;
    }

    Log& operator<<(const std::stringstream& sstr) {
        _tmp_stream << sstr.str();
        //std::cout << sstr.str();
        //_stream << sstr.str();
        return *this;
    }

    void operator<<(LogPrefix prefix) {
        _flush(prefix);
    }

private:
    void _flush(LogPrefix prefix) {
        std::string str = _message_prefixes[static_cast<int>(prefix)] + _tmp_stream.str();
        _tmp_stream.str(std::string());
        std::cout << str;
        _stream << str;
    }
};

