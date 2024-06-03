#include "reference_manager.h"


ReferenceManager::ReferenceManager() {

}

bool ReferenceManager::read_file(const std::filesystem::path& file) {
    if (file.extension() != ".csv") {
        bsclog::error("{} - not supported file format.", file.string());
        return false;
    }

    reset();

    try {
        io::CSVReader<2> source(file);
        source.read_header(io::ignore_extra_column, "duration", "reference");
        uint64_t duration;
        float reference;
        while(source.read_row(duration, reference)){
            _ref.push_back({std::chrono::milliseconds{duration}, reference});
        }
    } catch(const std::exception& e) {
        bsclog::error("{}", e.what());
        return false;
    }
    return true;
}