#pragma once

#include <imgui.h>
#include <memory>
#include <ucanopen_servers/shm80/shm80_server.h>
#include <ui/util/style.h>
#include <ui/view/view.h>

namespace ui {
namespace shm80 {

enum class MkfJs38Gear : uint8_t {
    neutral = 0x00,
    drive = 0x01,
    drive_pre = 0x02,
    reverse_pre = 0x04,
    reverse = 0x08
};

struct MkfJs38Message {
    uint8_t const fixed_value0{0x01};

    MkfJs38Gear gear;

    uint8_t const _reserved_byte2_{0};

    uint8_t throttle;

    uint8_t const _reserved_byte4_{0};

    uint8_t button : 1;
    uint8_t const _reserved_byte5_ : 7 {0};

    uint8_t const fixed_value6{0x00};

    uint8_t const fixed_value7{0xA5};

    // bool valid() const {
    //     const bool b0 = fixed_value0 == 0x01;
    //     const bool b1 = (gear == 0) || emb::ispow2(gear);
    //     const bool b3 = throttle <= 100;
    //     const bool b6 = fixed_value6 == 0x00;
    //     const bool b7 = fixed_value7 == 0xA5;
    //     return b0 && b1 && b3 && b6 && b7;
    // }
};

static_assert(sizeof(MkfJs38Message) == 8);

class ThrottleController : public View {
private:
    static constexpr canid_t can_id = 0x0CFA0001;

    std::shared_ptr<::shm80::Server> server_;
    std::chrono::time_point<std::chrono::steady_clock> send_timepoint_;
    std::chrono::milliseconds send_period_{10};

    bool enabled_{false};

    uint8_t throttle_{0};
    MkfJs38Gear gear_{MkfJs38Gear::neutral};
    int gear_v_{std::to_underlying(MkfJs38Gear::neutral)};
    int throttle_v_{0};
public:
    ThrottleController(std::shared_ptr<::shm80::Server> server,
                       const std::string& menu_title,
                       const std::string& window_title,
                       bool open);
    virtual void draw() override;
private:
    void send_frame();
};

} // namespace shm80
} // namespace ui
