#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen/server/server.h>

#include <memory>
#include <string>


namespace ui {


// The PDOs of a server as its description states them: what each TPDO
// brought last and what each RPDO sent last, signal by signal.
class PdoPanel : public View {
private:
    std::shared_ptr<ucanopen::Server> _server;
public:
    PdoPanel(std::shared_ptr<ucanopen::Server> server,
             const std::string& menu_title,
             const std::string& window_title,
             bool open);
    virtual void draw() override;
private:
    void _draw_pdo(const ucanopen::od::Description& description, const ucanopen::od::Pdo& pdo);
};


} // namespace ui
