#include "server_info_service.h"

#include <ucanopen/od/value.h>

#include <map>
#include <set>


namespace ucanopen {


ServerInfoService::ServerInfoService(impl::Server& server,
                                     ServerSdoService& sdo_service,
                                     const ServerHeartbeatService& heartbeat)
        : _server(server)
        , _heartbeat(heartbeat)
        , _sequence(sdo_service) {
    const auto& description = _server.description();
    if (!description) {
        return;
    }

    // objects and texts by key, as the description orders them
    std::map<od::Key, od::Item> items;
    for (const auto& [key, object] : description->objects()) {
        if (object.cls == od::Class::about) {
            if (auto item = description->resolve(key); item.has_value() && item->available) {
                items.emplace(key, *item);
            }
        }
    }
    for (const auto& [index, array] : description->arrays()) {
        if (array.cls == od::Class::about && array.text) {
            if (auto item = description->resolve({index, 0x00}); item.has_value()) {
                items.emplace(item->key, *item);
            }
        }
    }
    for (auto& [key, item] : items) {
        _items.push_back(std::move(item));
    }
    _fingerprint_key = description->find("dictionary.fingerprint");

    for (const auto& item : _items) {
        _info.entries.push_back({item, std::nullopt});
    }
}


ServerInfoService::Info ServerInfoService::info() const {
    std::lock_guard<std::mutex> lock(_mtx);
    return _info;
}


std::optional<bool> ServerInfoService::fingerprint_matches() const {
    std::lock_guard<std::mutex> lock(_mtx);
    if (!_info.fingerprint.has_value() || !_server.description()) {
        return std::nullopt;
    }
    return *_info.fingerprint == _server.description()->fingerprint();
}


void ServerInfoService::send() {
    if (_items.empty()) {
        return;
    }

    // A new connection, or a request: everything is read again.
    auto const epoch = _heartbeat.epoch();
    if (epoch != 0 && (epoch != _epoch || _refresh.exchange(false))) {
        _epoch = epoch;
        _start();
    }

    _sequence.send();
    if (!_taken && !_sequence.busy()) {
        _take();
        _taken = true;
    }
}


void ServerInfoService::_start() {
    _sequence.cancel();

    std::vector<SdoStep> steps;
    for (const auto& item : _items) {
        steps.push_back(SdoStep::read(item.key));
    }

    // A text reads its length at 00h first, then its characters, one a
    // request. The other objects of the class about lie at 00h too.
    std::set<uint16_t> texts;
    for (const auto& item : _items) {
        if (item.kind == od::Item::Kind::count) {
            texts.insert(item.key.index);
        }
    }
    _sequence.start(std::move(steps), [texts](const SdoStep& step) {
        std::vector<SdoStep> more;
        if (texts.contains(step.key.index) && step.key.subindex == 0x00 && step.status == SdoStep::Status::done) {
            for (unsigned i = 1; i <= step.value.u8(); ++i) {
                more.push_back(SdoStep::read({step.key.index, static_cast<uint8_t>(i)}));
            }
        }
        return more;
    });
    _taken = false;

    std::lock_guard<std::mutex> lock(_mtx);
    for (auto& entry : _info.entries) {
        entry.value.reset();
    }
    _info.fingerprint.reset();
    _info.reading = true;
}


void ServerInfoService::_take() {
    std::map<od::Key, SdoStep> done;
    for (const auto& step : _sequence.steps()) {
        if (step.status == SdoStep::Status::done) {
            done.emplace(step.key, step);
        }
    }

    Info info;
    for (const auto& item : _items) {
        Entry entry{item, std::nullopt};
        auto const step = done.find(item.key);
        if (step != done.end()) {
            if (item.kind == od::Item::Kind::count) {
                // the text whole, or nothing
                std::string text;
                bool whole = true;
                for (unsigned i = 1; i <= step->second.value.u8(); ++i) {
                    auto const character = done.find({item.key.index, static_cast<uint8_t>(i)});
                    if (character == done.end()) {
                        whole = false;
                        break;
                    }
                    text.push_back(static_cast<char>(character->second.value.u8()));
                }
                if (whole) {
                    entry.value = std::move(text);
                }
            } else {
                entry.value = od::format_value(item, step->second.value);
                if (_fingerprint_key == item.key) {
                    info.fingerprint = step->second.value.u32();
                }
            }
        }
        info.entries.push_back(std::move(entry));
    }

    if (info.fingerprint.has_value() && *info.fingerprint != _server.description()->fingerprint()) {
        bsclog::warning("{}: описание словаря не от этой прошивки: в устройстве отпечаток 0x{:08X}, в файле 0x{:08X}.",
                        _server.name(),
                        *info.fingerprint,
                        _server.description()->fingerprint());
    }

    std::lock_guard<std::mutex> lock(_mtx);
    _info = std::move(info);
}


} // namespace ucanopen
