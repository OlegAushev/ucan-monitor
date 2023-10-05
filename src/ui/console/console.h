#pragma once


#include <imgui.h>
#include <memory>
#include <sstream>
#include <vector>

namespace ui {


class Console {
private:
    Console() {
        _stream = std::make_shared<std::stringstream>();
    }
    std::shared_ptr<std::stringstream> _stream;
    std::vector<std::string> _lines;
public:
    static Console& instance() {
        static Console s;
        return s;
    }

    void draw();
    std::shared_ptr<std::stringstream> stream() { return _stream; }
};


} // namespace ui