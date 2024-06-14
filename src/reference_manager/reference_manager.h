#pragma once


#include <chrono>
#include <filesystem>
#include <optional>
#include <vector>
#include <fast-cpp-csv-parser/csv.h>
#include <bsclog/bsclog.h>


class ReferenceManager {
private:
    std::string _label;
    std::vector<std::pair<std::chrono::milliseconds, float>> _ref;
    mutable size_t _idx{0};
    mutable std::optional<std::chrono::time_point<std::chrono::steady_clock>> _timepoint;
public:
    ReferenceManager();

    std::string label() const { return _label; }

    void reset() {
        _label.clear();
        _ref.clear();
        _idx = 0;
        _timepoint.reset();
    }

    bool read_file(const std::filesystem::path& file);

    bool empty() const { return _ref.empty(); }

    float progress() const {
        if (_ref.size() == 0) {
            return 0;
        }
        return float(_idx)/float(_ref.size());
    }

    std::optional<float> get() const {
        if (!_timepoint.has_value()) {
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
