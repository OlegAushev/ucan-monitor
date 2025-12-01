#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <regex>

class CsvLogger {
public:
    struct Config {
        std::filesystem::path directory{"logs"};
        std::size_t max_file_size{10 * 1024 * 1024};
        std::size_t max_files{5};
    };

    explicit CsvLogger(Config config)
        : config_{std::move(config)}
    {
        std::filesystem::create_directories(config_.directory);
        scan_existing_files();
        open_new_file();
    }

    void write(std::string_view object, float value) {
        if (current_size_ >= config_.max_file_size) {
            rotate();
        }

        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        auto time = std::chrono::system_clock::to_time_t(now);

        std::tm tm{};
        localtime_r(&time, &tm);

        file_ << std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03},{},{}",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec, ms.count(),
            object, value) << '\n';

        current_size_ = file_.tellp();
    }

    void flush() { file_.flush(); }

private:
    std::filesystem::path make_path(std::size_t seq) const {
        return config_.directory / std::format("{:06}.csv", seq);
    }

    void scan_existing_files() {
        if (!std::filesystem::exists(config_.directory)) return;

        std::vector<std::size_t> existing;
        for (const auto& entry : std::filesystem::directory_iterator(config_.directory)) {
            if (entry.path().extension() == ".csv") {
                auto stem = entry.path().stem().string();
                if (stem.size() == 6) {
                    try {
                        existing.push_back(std::stoull(stem));
                    } catch (...) {}
                }
            }
        }

        std::sort(existing.begin(), existing.end());
        for (auto seq : existing) {
            file_sequence_.push_back(seq);
        }

        if (!existing.empty()) {
            next_sequence_ = existing.back() + 1;
        }
    }

    void open_new_file() {
        file_.open(make_path(next_sequence_), std::ios::out | std::ios::trunc);
        file_ << "timestamp,object,value\n";
        file_sequence_.push_back(next_sequence_);
        ++next_sequence_;
        current_size_ = file_.tellp();
    }

    void rotate() {
        file_.close();

        while (file_sequence_.size() >= config_.max_files) {
            auto oldest = file_sequence_.front();
            file_sequence_.pop_front();
            std::filesystem::remove(make_path(oldest));
        }

        open_new_file();
    }

    Config config_;
    std::ofstream file_;
    std::deque<std::size_t> file_sequence_;
    std::size_t next_sequence_{0};
    std::size_t current_size_{0};
};
