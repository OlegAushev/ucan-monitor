#pragma once


#include <chrono>
#include <format>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <utility>


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


const std::vector<std::string> level_name = {
    " trace ",
    " debug ",
    " info ",
    "  ok  ",
    " warn ",
    " fail ",
    " crit ",
    "      "
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
    void trace(std::format_string<Args...> fmt, Args&&... args) {
        _log(level::trace, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(std::format_string<Args...> fmt, Args&&... args) {
        _log(level::debug, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        _log(level::info, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void success(std::format_string<Args...> fmt, Args&&... args) {
        _log(level::success, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warning(std::format_string<Args...> fmt, Args&&... args) {
        _log(level::warning, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) {
        _log(level::error, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(std::format_string<Args...> fmt, Args&&... args) {
        _log(level::critical, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log(std::format_string<Args...> fmt, Args&&... args) {
        _log(level::off, fmt, std::forward<Args>(args)...);
    }

private:
    template <typename... Args>
    void _log(level lvl, std::format_string<Args...> fmt, Args&&... args) {
        // auto now = std::chrono::system_clock::now();
        // auto now_time_t = std::chrono::system_clock::to_time_t(now);
        // std::stringstream timestamp;
        // timestamp << '[' << std::put_time(std::localtime(&now_time_t), "%F %T") << '] ';
 
        std::string prefix;
        if (lvl != level::off) {
            prefix = std::format("[{}] ",
                                level_name[std::to_underlying(lvl)]);
        } else {
            prefix = std::format(" {}  ",
                                level_name[std::to_underlying(lvl)]);
        }

        std::string payload = std::format(fmt, std::forward<Args>(args)...);
        
        for (auto sink : _sinks) {
            *sink << (prefix + payload) << '\n';
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
inline void trace(std::format_string<Args...> fmt, Args&&... args) {
    logger::instance().trace(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void debug(std::format_string<Args...> fmt, Args&&... args) {
    logger::instance().debug(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void info(std::format_string<Args...> fmt, Args&&... args) {
    logger::instance().info(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void success(std::format_string<Args...> fmt, Args&&... args) {
    logger::instance().success(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void warning(std::format_string<Args...> fmt, Args&&... args) {
    logger::instance().warning(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void error(std::format_string<Args...> fmt, Args&&... args) {
    logger::instance().error(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void critical(std::format_string<Args...> fmt, Args&&... args) {
    logger::instance().critical(fmt, std::forward<Args>(args)...);
}


template <typename... Args>
inline void log(std::format_string<Args...> fmt, Args&&... args) {
    logger::instance().log(fmt, std::forward<Args>(args)...);
}


} // namespace bsclog
