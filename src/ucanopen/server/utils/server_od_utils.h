#pragma once


#include <ucanopen/server/impl/impl_server.h>
#include <ucanopen/server/services/sdo/server_sdo_service.h>
#include <future>
#include <optional>
#include <vector>


namespace ucanopen {
namespace utils {


class StringReader : public SdoSubscriber {
private:
    ServerSdoService& _sdo_service;
    ODEntryIter _entry;

    std::vector<char> _charbuf;
    std::optional<std::string> _result;
    bool _ready = false;
public:
    StringReader(ServerSdoService& sdo_service,
                 std::string_view category, std::string_view subcategory, std::string_view name);
    std::optional<std::string> get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};


class ScalarReader : public SdoSubscriber {
private:
    ServerSdoService& _sdo_service;
    ODEntryIter _entry;

    std::optional<std::string> _result;
    bool _ready = false;
public:
    ScalarReader(ServerSdoService& sdo_service,
                 std::string_view category, std::string_view subcategory, std::string_view name);
    std::optional<std::string> get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};


class ExpeditedSdoDataReader : public SdoSubscriber {
private:
    ServerSdoService& _sdo_service;
    ODEntryIter _entry;

    std::optional<ExpeditedSdoData> _result;
    bool _ready = false;
public:
    ExpeditedSdoDataReader(ServerSdoService& sdo_service,
                           std::string_view category, std::string_view subcategory, std::string_view name);
    std::optional<ExpeditedSdoData> get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};


class Executor : public SdoSubscriber {
private:
    ServerSdoService& _sdo_service;
    ODEntryIter _entry;

    std::optional<ExpeditedSdoData> _result;
    bool _ready = false;
public:
    Executor(ServerSdoService& sdo_service,
             std::string_view category, std::string_view subcategory, std::string_view name);
    std::optional<ExpeditedSdoData> get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};


class ExpeditedSdoDataWriter : public SdoSubscriber {
private:
    ServerSdoService& _sdo_service;
    ODEntryIter _entry;

    std::optional<ExpeditedSdoData> _result;
    bool _ready = false;
public:
    ExpeditedSdoDataWriter(ServerSdoService& sdo_service,
                           std::string_view category, std::string_view subcategory, std::string_view name,
                           ExpeditedSdoData sdo_data);
    std::optional<ExpeditedSdoData> get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};


} // namespace utils
} // namespace ucanopen 
