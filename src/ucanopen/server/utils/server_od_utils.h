#pragma once


#include "../impl/impl_server.h"
#include <future>
#include <optional>
#include <vector>


namespace ucanopen {
namespace utils {


class StringReader : public SdoSubscriber {
private:
    impl::Server& _server;
    ODEntryIter _entry;

    std::vector<char> _charbuf;
    std::optional<std::string> _result;
    bool _ready = false;
public:
    StringReader(impl::Server& server, impl::SdoPublisher& publisher,
                 std::string_view category, std::string_view subcategory, std::string_view name);
    std::optional<std::string> get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};


class ScalarReader : public SdoSubscriber {
private:
    impl::Server& _server;
    ODEntryIter _entry;

    std::optional<std::string> _result;
    bool _ready = false;
public:
    ScalarReader(impl::Server& server, impl::SdoPublisher& publisher,
                 std::string_view category, std::string_view subcategory, std::string_view name);
    std::optional<std::string> get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};


class ExpeditedSdoDataReader : public SdoSubscriber {
private:
    impl::Server& _server;
    ODEntryIter _entry;

    std::optional<ExpeditedSdoData> _result;
    bool _ready = false;
public:
    ExpeditedSdoDataReader(impl::Server& server, impl::SdoPublisher& publisher,
                           std::string_view category, std::string_view subcategory, std::string_view name);
    std::optional<ExpeditedSdoData> get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};


class Executor : public SdoSubscriber {
private:
    impl::Server& _server;
    ODEntryIter _entry;

    std::optional<ExpeditedSdoData> _result;
    bool _ready = false;
public:
    Executor(impl::Server& server, impl::SdoPublisher& publisher,
             std::string_view category, std::string_view subcategory, std::string_view name);
    std::optional<ExpeditedSdoData> get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};


class ExpeditedSdoDataWriter : public SdoSubscriber {
private:
    impl::Server& _server;
    ODEntryIter _entry;

    std::optional<ExpeditedSdoData> _result;
    bool _ready = false;
public:
    ExpeditedSdoDataWriter(impl::Server& server, impl::SdoPublisher& publisher,
                           std::string_view category, std::string_view subcategory, std::string_view name,
                           ExpeditedSdoData sdo_data);
    std::optional<ExpeditedSdoData> get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};


} // namespace utils
} // namespace ucanopen 
