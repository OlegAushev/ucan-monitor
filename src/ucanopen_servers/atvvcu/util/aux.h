#pragma once


#include <atomic>


namespace atvvcu {


class Server;


namespace aux {


class Aux {
    friend class atvvcu::Server;
private:
    std::atomic_bool _pump_enabled{false};
    std::atomic_bool _hydrostation_enabled{false};
    std::atomic_bool _fan_enabled{false};

public:
    bool pump_enabled() const { return _pump_enabled.load(); }
    bool hydrostation_enabled() const { return _hydrostation_enabled.load(); }
    bool fan_enabled() const { return _fan_enabled.load(); }
};


} // namespace aux


} // namespace atvvcu
