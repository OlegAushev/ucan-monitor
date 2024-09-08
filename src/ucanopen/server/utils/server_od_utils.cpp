#include "server_od_utils.h"


namespace ucanopen {
namespace utils {


//----------------------------------------------------------------------------------------------------------------------
StringReader::StringReader(ServerSdoService& sdo_service,
                           std::string_view category, std::string_view subcategory, std::string_view name)
        : SdoSubscriber(sdo_service)
        , _sdo_service(sdo_service)
{
    if (_sdo_service.server().find_od_entry_to_read(category, subcategory, name, _entry) != ODAccessStatus::success) {
        _ready = true;
        return;
    }

    const auto& [key, object] = *_entry;

    if (object.data_type != OD_STRING) {
        _ready = true;
        return;
    }

    if (_sdo_service.read(object.category, object.subcategory, object.name) != ODAccessStatus::success) {
        _ready = true;
        return;
    }
}


std::optional<std::string> StringReader::get(std::future<void> signal_terminate) const {
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
            _sdo_service.read(object.category, object.subcategory, object.name);
        }

        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}


//----------------------------------------------------------------------------------------------------------------------
ScalarReader::ScalarReader(ServerSdoService& sdo_service,
                           std::string_view category, std::string_view subcategory, std::string_view name)
        : SdoSubscriber(sdo_service)
        , _sdo_service(sdo_service)
{
    if (_sdo_service.server().find_od_entry_to_read(category, subcategory, name, _entry) != ODAccessStatus::success) {
        _ready = true;
        return;
    }

    const auto& [key, object] = *_entry;

    if (object.data_type == OD_EXEC || object.data_type == OD_STRING) {
        _ready = true;
        return;
    }

    if (_sdo_service.read(category, subcategory, name) != ODAccessStatus::success) {
        _ready = true;
        return;
    }	
}


std::optional<std::string> ScalarReader::get(std::future<void> signal_terminate) const {
    while (signal_terminate.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout
       && !_ready) {
        /*WAIT*/
    }
    return _result;
}


FrameHandlingStatus ScalarReader::handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) {
    if (entry == _entry) {
        if (sdo_type == SdoType::response_to_read) {
            _result = sdo_data.to_string(_entry->second.data_type);
        }
        _ready = true;
        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}


//----------------------------------------------------------------------------------------------------------------------
ExpeditedSdoDataReader::ExpeditedSdoDataReader(ServerSdoService& sdo_service,
                                               std::string_view category, std::string_view subcategory, std::string_view name)
        : SdoSubscriber(sdo_service)
        , _sdo_service(sdo_service)
{
    if (_sdo_service.server().find_od_entry_to_read(category, subcategory, name, _entry) != ODAccessStatus::success) {
        _ready = true;
        return;
    }

    const auto& [key, object] = *_entry;

    if (object.data_type == OD_EXEC || object.data_type == OD_STRING) {
        _ready = true;
        return;
    }

    if (_sdo_service.read(category, subcategory, name) != ODAccessStatus::success) {
        _ready = true;
        return;
    }	
}


std::optional<ExpeditedSdoData> ExpeditedSdoDataReader::get(std::future<void> signal_terminate) const {
    while (signal_terminate.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout
       && !_ready) {
        /*WAIT*/
    }
    return _result;
}


FrameHandlingStatus ExpeditedSdoDataReader::handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) {
    if (entry == _entry) {
        if (sdo_type == SdoType::response_to_read) {
            _result = sdo_data;
        }
        _ready = true;
        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}


//----------------------------------------------------------------------------------------------------------------------
Executor::Executor(ServerSdoService& sdo_service,
                   std::string_view category, std::string_view subcategory, std::string_view name)
        : SdoSubscriber(sdo_service)
        , _sdo_service(sdo_service)\
{
    if (_sdo_service.exec(category, subcategory, name) != ODAccessStatus::success) {
        _ready = true;
        return;
    }	
}


std::optional<ExpeditedSdoData> Executor::get(std::future<void> signal_terminate) const {
    while (signal_terminate.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout
       && !_ready) {
        /*WAIT*/
    }
    return _result;
}


FrameHandlingStatus Executor::handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) {
    if (entry == _entry) {
        if (sdo_type == SdoType::response_to_exec) {
            _result = sdo_data;
        }
        _ready = true;
        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}


//----------------------------------------------------------------------------------------------------------------------
ExpeditedSdoDataWriter::ExpeditedSdoDataWriter(ServerSdoService& sdo_service,
                                               std::string_view category, std::string_view subcategory, std::string_view name,
                                               ExpeditedSdoData sdo_data)
        : SdoSubscriber(sdo_service)
        , _sdo_service(sdo_service)
{
    if (_sdo_service.server().find_od_entry_to_write(category, subcategory, name, _entry) != ODAccessStatus::success) {
        _ready = true;
        return;
    }

    const auto& [key, object] = *_entry;

    if (object.data_type == OD_EXEC || object.data_type == OD_STRING) {
        _ready = true;
        return;
    }

    if (_sdo_service.write(category, subcategory, name, sdo_data) != ODAccessStatus::success) {
        _ready = true;
        return;
    }	
}


std::optional<ExpeditedSdoData> ExpeditedSdoDataWriter::get(std::future<void> signal_terminate) const {
    while (signal_terminate.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout
       && !_ready) {
        /*WAIT*/
    }
    return _result;
}


FrameHandlingStatus ExpeditedSdoDataWriter::handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) {
    if (entry == _entry) {
        if (sdo_type == SdoType::response_to_write) {
            _result = sdo_data;
        }
        _ready = true;
        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}


} // namespace utils
} // namespace ucanopen
