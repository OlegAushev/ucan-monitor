#pragma once


#include <fstream>


class CsvWriter {
private:
    std::unique_ptr<std::ofstream> _file;
    std::string _separator;
    bool _first_row{true};
public:
    CsvWriter(const std::string& filename, const std::string& separator = ";") {
        _file = std::make_unique<std::ofstream>(filename);
        _separator = separator;
    }

    CsvWriter& new_row() {
        return *this;
    }


};
