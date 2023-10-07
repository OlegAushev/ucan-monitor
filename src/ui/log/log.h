#pragma once


#include <imgui.h>
#include <memory>
#include <sstream>
#include <vector>

namespace ui {


class Log {
private:
    std::shared_ptr<std::stringstream> _stream;
    std::vector<std::string> _lines;
public:
    Log();
    void draw(bool& open);
    std::shared_ptr<std::stringstream> stream() { return _stream; }
};


} // namespace ui
