#pragma once


#include <chrono>
#ifdef __cpp_lib_format
#include <format>
namespace bsclog_fmt = std;
#else
#include <fmt/format.h>
#include <fmt/chrono.h>
namespace bsclog_fmt = fmt;
#endif
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <utility>


#ifndef __cpp_lib_to_underlying
namespace std {
template<typename E>
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept {
    static_assert(std::is_enum_v<E> == true);
    return static_cast<std::underlying_type_t<E>>(e);
}
}
#endif


namespace bsclog {


enum class level {
    trace,
    debug,
    info,
    success,
    warning,
    error,
    critical,
    off
};


const std::vector<std::string> level_prefix = {
    "[ trace ]",
    "[ debug ]",
    "        ",
    "[  ok  ]",
    "[ warn ]",
    "[ fail ]",
    "[ crit ]",
    "        "
};


class logger {
private:
    logger() = default;
    std::set<std::shared_ptr<std::ostream>> _sinks;
public:
    static logger& instance() {
        static logger _logger;
        return _logger;
    }

    auto sink_count() const { return _sinks.size(); }

    void add_sink(std::shared_ptr<std::ostream> sink) {
        _sinks.insert(sink);
    }

    template <typename... Args>
    void trace(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
        _log(level::trace, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
        _log(level::debug, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
        _log(level::info, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void success(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
        _log(level::success, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warning(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
        _log(level::warning, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
        _log(level::error, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
        _log(level::critical, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
        _log(level::off, fmt, std::forward<Args>(args)...);
    }

private:
    template <typename... Args>
    void _log(level lvl, bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
        // auto now = std::chrono::system_clock::now();
        // auto now_time_t = std::chrono::system_clock::to_time_t(now);
        // std::stringstream timestamp;
        // timestamp << '[' << std::put_time(std::localtime(&now_time_t), "%F %T") << '] ';

        std::string payload = bsclog_fmt::format(fmt, std::forward<Args>(args)...);
        std::string message = bsclog_fmt::format("{} {}", level_prefix[std::to_underlying(lvl)], payload); 

        for (auto sink : _sinks) {
            *sink << message << '\n';
        }
    }

private:
    struct message {
        std::chrono::time_point<std::chrono::system_clock> time;
        level lvl;
        std::string_view payload;
    };
};


inline void add_sink(std::shared_ptr<std::ostream> sink) {
    logger::instance().add_sink(sink);
}


inline auto sink_count() {
    return logger::instance().sink_count();
}


template <typename... Args>
inline void trace(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
    logger::instance().trace(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void debug(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
    logger::instance().debug(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void info(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
    logger::instance().info(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void success(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
    logger::instance().success(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void warning(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
    logger::instance().warning(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void error(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
    logger::instance().error(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void critical(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
    logger::instance().critical(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void log(bsclog_fmt::format_string<Args...> fmt, Args&&... args) {
    logger::instance().log(fmt, std::forward<Args>(args)...);
}


} // namespace bsclog
