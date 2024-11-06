#pragma once


#include <fstream>
#include <iomanip>


class CsvWriter {
private:
    std::ofstream _file;
    std::string _separator;
public:
    CsvWriter(const std::string& filename, const std::string& separator = ";")
            : _file(filename)
            , _separator(separator)
    {}

    CsvWriter& operator<<(float t) {
        _file << std::setprecision(6) << t << _separator;
        return *this;
    }

    template <typename T>
    CsvWriter& operator<<(const T& t) {
        _file << t << _separator;
        return *this;
    }

    void next_row() {
        _file << '\n';
    }
};
