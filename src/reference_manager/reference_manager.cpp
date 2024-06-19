#include "reference_manager.h"


ReferenceManager::ReferenceManager(const std::string& ref_unit) : _ref_unit(ref_unit) {}


bool ReferenceManager::read_file(const std::filesystem::path& file) {
    if (file.extension() != ".csv") {
        bsclog::error("{} - not supported file format.", file.string());
        return false;
    }

    reset();

    try {
        io::CSVReader<2> source(file);
        source.read_header(io::ignore_extra_column, "duration[ms]", _ref_unit);
        uint64_t duration;
        float reference;
        while(source.read_row(duration, reference)){
            _ref.push_back({std::chrono::milliseconds{duration}, reference});
            _total_time = _total_time.value_or(std::chrono::milliseconds{0})
                    + std::chrono::milliseconds{duration};
        }
    } catch(const std::exception& e) {
        bsclog::error("{}", e.what());
        return false;
    }

    _label = file.stem();

    return true;
}