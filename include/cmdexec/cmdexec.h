#pragma once


#include <array>
#include <memory>
#include <ostream>
#include <string>


namespace cmdexec {


struct result {
    std::string output;
    int exitcode;
};


inline result run(const std::string& cmd) {
    int exitcode = 0;
    std::string output;
    std::array<char, 1024> buf = {};

    FILE *pipe = popen(cmd.c_str(), "r");

    if (pipe == nullptr) {
        throw std::runtime_error("popen() failed!");
    }
    
    try {
        std::size_t bytesread;
        while ((bytesread = std::fread(buf.data(), sizeof(buf.at(0)), sizeof(buf), pipe)) != 0) {
            output += std::string(buf.data(), bytesread);
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    exitcode = WEXITSTATUS(pclose(pipe));
    return result{output, exitcode};
}


} // namespace cmdexec
