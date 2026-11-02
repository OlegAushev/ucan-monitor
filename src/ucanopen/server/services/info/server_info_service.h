#pragma once


#include <ucanopen/od/description.h>
#include <ucanopen/server/impl/impl_server.h>
#include <ucanopen/server/services/heartbeat/server_heartbeat_service.h>
#include <ucanopen/server/services/sdo/server_sdo_service.h>
#include <ucanopen/server/utils/sdo_sequence.h>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <vector>


namespace ucanopen {


// What a device says of itself: the objects and the texts of the class about
// in its description, its fingerprint, name, serial number, versions and
// build among them. The CAN thread reads them once a connection, a text a
// character at a time, so that no panel waits for the bus, and checks the
// fingerprint against the description's.
class ServerInfoService {
public:
    struct Entry {
        od::Item item;                    // an object, or the count of a text
        std::optional<std::string> value; // the text, or the value as format_value shows it; none if unread
    };

    struct Info {
        std::vector<Entry> entries; // in the order of the description
        std::optional<uint32_t> fingerprint;
        bool reading{false};
    };
private:
    impl::Server& _server;
    const ServerHeartbeatService& _heartbeat;
    SdoSequence _sequence;
    std::vector<od::Item> _items;
    std::optional<od::Key> _fingerprint_key;

    // The CAN thread's own: the connection read and whether it took the
    // result.
    unsigned _epoch{0};
    bool _taken{true};
    std::atomic<bool> _refresh{false};

    mutable std::mutex _mtx;
    Info _info;
public:
    ServerInfoService(impl::Server& server, ServerSdoService& sdo_service, const ServerHeartbeatService& heartbeat);

    Info info() const;

    // Whether the device's fingerprint is the description's; none until it
    // is read.
    std::optional<bool> fingerprint_matches() const;

    // Reads everything again.
    void refresh() { _refresh = true; }

    void send();
private:
    void _start();
    void _take();
};


} // namespace ucanopen
