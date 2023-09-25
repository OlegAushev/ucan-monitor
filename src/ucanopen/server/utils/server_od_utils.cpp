#include "server_od_utils.h"


namespace ucanopen {

namespace utils {

uint32_t SerialNumberReader::get(std::future<void> signal_terminate) const {
    _server.read("sys", "info", "serial_number");
    while (signal_terminate.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout
        && _serial_number == 0) {
        /*WAIT*/
    }
    return _serial_number;
}


FrameHandlingStatus SerialNumberReader::handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) {
    if (sdo_type == SdoType::response_to_read
     && entry->second.category == "sys"
     && entry->second.subcategory == "info"
     && entry->second.name == "serial_number") {
        _serial_number = sdo_data.u32();
        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}


StringReader::StringReader(impl::Server& server, impl::SdoPublisher& publisher,
                           std::string_view category, std::string_view subcategory, std::string_view name)
        : SdoSubscriber(publisher)
        , _server(server) {
    if (_server.find_od_entry(category, subcategory, name, _entry, traits::check_read_perm{}) != ODAccessStatus::success) {
        _ready = true;
        return;
    }

    const auto& [key, object] = *_entry;

    if (object.type != OD_STRING) {
        _ready = true;
        return;
    }

    if (_server.read(object.category, object.subcategory, object.name) != ODAccessStatus::success) {
        _ready = true;
        return;
    }
}


std::string StringReader::get(std::future<void> signal_terminate) const {
    while (signal_terminate.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout
       && !_ready) {
        /*WAIT*/
    }
    return _result;
}

FrameHandlingStatus StringReader::handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) {
    if (sdo_type == SdoType::response_to_read && entry == _entry) {
        std::array<char, 4> char_arr;
        memcpy(char_arr.data(), &sdo_data, 4);
        for (auto ch : char_arr) {
            _charbuf.push_back(ch);
            if (ch == '\0') {
                _result = std::string(_charbuf.data());
                _ready = true;
                break;
            }
        }

        if (!_ready) {
            const auto& [key, object] = *_entry;
            _server.read(object.category, object.subcategory, object.name);
        }

        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}


NumvalReader::NumvalReader(impl::Server& server, impl::SdoPublisher& publisher,
                           std::string_view category, std::string_view subcategory, std::string_view name)
        : SdoSubscriber(publisher)
        , _server(server) {
    if (_server.find_od_entry(category, subcategory, name, _entry, traits::check_read_perm{}) != ODAccessStatus::success) {
        _ready = true;
        return;
    }

    const auto& [key, object] = *_entry;

    if (object.type == OD_EXEC || object.type == OD_STRING) {
        _ready = true;
        return;
    }

    if (_server.read(category, subcategory, name) != ODAccessStatus::success) {
        _ready = true;
        return;
    }	
}


std::string NumvalReader::get(std::future<void> signal_terminate) const {
    while (signal_terminate.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout
       && !_ready) {
        /*WAIT*/
    }
    return _result;
}


FrameHandlingStatus NumvalReader::handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) {
    if (sdo_type == SdoType::response_to_read && entry == _entry) {
        _result = sdo_data.to_string(_entry->second.type);
        _ready = true;
        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}

} // namespace utils

} // namespace ucanopen

