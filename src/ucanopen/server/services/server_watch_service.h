#pragma once


#include "../impl/impl_server.h"
#include <vector>
#include <map>
#include <mutex>


namespace ucanopen {


class ServerWatchService : public SdoSubscriber {
private:
    impl::Server& _server;
    bool _is_enabled = false;
    std::chrono::milliseconds _period = std::chrono::milliseconds(1000);
    std::chrono::time_point<std::chrono::steady_clock> _timepoint;
    std::vector<std::pair<std::string_view, std::string_view>> _object_list;

    struct WatchData {
        ExpeditedSdoData raw;
        std::string str;
    };

    std::map<std::pair<std::string_view, std::string_view>, WatchData> _data;
    mutable std::mutex _data_mtx;
public:
    ServerWatchService(impl::Server& server, impl::SdoPublisher& sdo_publisher);

    void send() {
        if (_is_enabled && !_object_list.empty()) {
            auto now = std::chrono::steady_clock::now();
            if (now - _timepoint >= _period) {
                static int i = 0;
                _server.read(_server.dictionary().config.watch_category,
                              _object_list[i].first,
                              _object_list[i].second);
                _timepoint = now;
                i = (i + 1) % _object_list.size();
            }
        }
    }

    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override {
        std::lock_guard<std::mutex> lock(_data_mtx);
        const auto& [key, object] = *entry;

        if ((object.category == _server.dictionary().config.watch_category) && (sdo_type == SdoType::response_to_read)) {
            _data[std::make_pair(object.subcategory, object.name)].raw = sdo_data;
            if (object.type != OD_ENUM16) {
                _data[std::make_pair(object.subcategory, object.name)].str = sdo_data.to_string(object.type, 2);
            }
            return FrameHandlingStatus::success;
        }
        return FrameHandlingStatus::irrelevant_frame;
    }

    void enable() {
        _is_enabled = true;
        Log() << "Enabled uCANopen server {" << _server.name() << "} watch requests (period = " << _period << ").\n" << LogPrefix::ok;
    }

    void disable() {
        _is_enabled = false;
        Log() << "Disabled uCANopen server {" << _server.name() << "} watch requests.\n" << LogPrefix::ok;
    }

    void set_period(std::chrono::milliseconds period) {
        _period = period;
        Log() << "Set uCANopen server {" << _server.name() << "} watch requests period = " << period << ".\n" << LogPrefix::ok;
    }

    std::vector<std::pair<std::string_view, std::string_view>> object_list() const {
        return _object_list;
    }

    std::string value_str(std::string_view watch_subcategory, std::string_view watch_name) const {
        std::lock_guard<std::mutex> lock(_data_mtx);
        auto it = _data.find(std::make_pair(watch_subcategory, watch_name));
        if (it == _data.end()) {
            return "n/a";
        }
        return it->second.str;
    }

    void value_cstr(std::string_view watch_subcategory, std::string_view watch_name, char* retbuf, int bufsize) const {
        std::lock_guard<std::mutex> lock(_data_mtx);
        retbuf[0] = '\0';
        auto it = _data.find(std::make_pair(watch_subcategory, watch_name));
        if (it == _data.end()) {
            const char* str = "n/a";
            std::strncat(retbuf, str, bufsize-1);
            return;
        }
        std::strncat(retbuf, it->second.str.c_str(), bufsize-1);
    }

    ExpeditedSdoData value(std::string_view watch_subcategory, std::string_view watch_name) {
        std::lock_guard<std::mutex> lock(_data_mtx);
        auto it = _data.find(std::make_pair(watch_subcategory, watch_name));
        if (it == _data.end()) {
            return 0;
        }
        return it->second.raw;
    }

    void set_value_str(std::string_view watch_subcategory, std::string_view watch_name, std::string value_str) {
        std::lock_guard<std::mutex> lock(_data_mtx);
        auto it = _data.find(std::make_pair(watch_subcategory, watch_name));
        if (it == _data.end()) {
            return;
        }
        it->second.str = value_str;
    }
};


} // namespace ucanopen
