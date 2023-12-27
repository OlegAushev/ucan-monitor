#pragma once


#include <imgui.h>
#include <ui/view/view.h>

#include <memory>
#include <sstream>
#include <vector>


namespace ui {


class Log : public View {
private:
    std::shared_ptr<std::stringstream> _stream;
    std::vector<std::string> _lines;
public:
    Log();
    void draw();
    std::shared_ptr<std::stringstream> stream() { return _stream; }
};


} // namespace ui
