#pragma once


#include "../impl/impl_server.h"
#include <vector>
#include <future>


namespace ucanopen {

namespace utils {

class SerialNumberReader : public SdoSubscriber {
private:
    impl::Server& _server;
    uint32_t _serial_number = 0;
public:
    SerialNumberReader(impl::Server& server, impl::SdoPublisher& publisher)
            : SdoSubscriber(publisher)
            , _server(server)
    {}
    uint32_t get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};


class StringReader : public SdoSubscriber {
private:
    impl::Server& _server;
    ODEntryIter _entry;

    std::vector<char> _charbuf;
    std::string _result = "n/a";
    bool _ready = false;
public:
    StringReader(impl::Server& server, impl::SdoPublisher& publisher,
                 std::string_view category, std::string_view subcategory, std::string_view name);
    std::string get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};


class NumvalReader : public SdoSubscriber {
private:
    impl::Server& _server;
    ODEntryIter _entry;

    std::string _result = "n/a";
    bool _ready = false;
public:
    NumvalReader(impl::Server& server, impl::SdoPublisher& publisher,
                 std::string_view category, std::string_view subcategory, std::string_view name);
    std::string get(std::future<void> signal_terminate) const;
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
};

}

} // namespace ucanopen 

