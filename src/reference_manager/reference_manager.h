#pragma once


#include <chrono>
#include <filesystem>
#include <optional>
#include <vector>
#include <fast-cpp-csv-parser/csv.h>
#include <bsclog/bsclog.h>


class ReferenceManager {
private:
    std::string _ref_unit;

    std::string _label;
    std::vector<std::pair<std::chrono::milliseconds, float>> _ref;
    mutable size_t _idx{0};
    mutable std::optional<std::chrono::time_point<std::chrono::steady_clock>> _timepoint;

    mutable std::optional<std::chrono::time_point<std::chrono::steady_clock>> _start;
    std::optional<std::chrono::milliseconds> _total_time{0};
public:
    ReferenceManager(const std::string& ref_unit);

    std::string label() const { return _label; }

    void reset() {
        _label.clear();
        _ref.clear();
        _idx = 0;
        _timepoint.reset();
        _start.reset();
        _total_time.reset();
    }

    void restart() {
        _idx = 0;
        _timepoint.reset();
        _start.reset();
    }

    bool read_file(const std::filesystem::path& file);

    bool empty() const { return _ref.empty(); }

    float progress() const {
        if (_ref.size() == 0 || !_start.has_value() || !_total_time.has_value()) {
            return 0;
        }
        auto elapsed = std::chrono::steady_clock::now() - _start.value();
        return float(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count())
                / float(_total_time.value().count());
    }

    std::optional<float> get() const {
        if (!_timepoint.has_value()) {
            _start = std::chrono::steady_clock::now();
            _timepoint = std::chrono::steady_clock::now();
            _idx = 0;
        }

        if (_idx >= _ref.size()) {
            return std::nullopt;
        }                

        if (std::chrono::steady_clock::now() > _timepoint.value() + _ref[_idx].first) {
            _timepoint = std::chrono::steady_clock::now();
            ++_idx;
        }

        return _ref[_idx].second;
    } 
};
