#pragma once


#include <ucanopen/od/description.h>
#include <ucanopen/server/impl/impl_server.h>

#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <vector>


namespace ucanopen {


// The statuses of a device, which it sends itself: the members of the record
// of the class status in its description, each at a severity of the table of
// the members, which an MPDO in source address mode brings round one a frame
// (emblib, docs/canopen-od-algorithm.ru.md, section 4.10). A status whose
// frame has not come yet has no severity.
class ServerStatusService {
public:
    struct Status {
        const od::Field* field;          // the name, the label and the table of severities
        std::optional<uint8_t> severity; // none until its frame comes
    };
private:
    impl::Server& _server;
    const od::Record* _record{nullptr};
    const od::Pdo* _pdo{nullptr};
    std::chrono::milliseconds _timeout{500};

    mutable std::mutex _mtx;
    std::vector<Status> _statuses; // by subindex
    std::chrono::time_point<std::chrono::steady_clock> _timepoint{};
public:
    explicit ServerStatusService(impl::Server& server);

    // Whether the description states the statuses and the MPDO that sends
    // them.
    bool available() const { return _record != nullptr && _pdo != nullptr; }
    const od::Record* record() const { return _record; }
    const od::Pdo* pdo() const { return _pdo; }
    std::chrono::milliseconds timeout() const { return _timeout; }

    // Whether a frame of the MPDO came within the timeout.
    bool good() const;
    std::vector<Status> statuses() const;

    // A frame of the MPDO, which the CAN thread hands over.
    void handle_frame(const can_payload& payload);
};


} // namespace ucanopen
