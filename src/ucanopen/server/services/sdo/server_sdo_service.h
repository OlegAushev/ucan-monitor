#pragma once


#include <ucanopen/od/description.h>
#include <ucanopen/server/impl/impl_server.h>
#include <mutex>
#include <optional>
#include <set>
#include <string_view>


namespace ucanopen {


// An answer of the server to an SDO request: the key it answers for, what
// stands there by the description, if it knows the key, and the data.
struct SdoAnswer {
    od::Key key;
    std::optional<od::Item> item;
    SdoType type;          // response_to_read, response_to_write or abort
    ExpeditedSdoData data; // the code of an abort in u32()
};


class SdoSubscriber;


class SdoPublisher {
public:
    virtual ~SdoPublisher() = default;
    void register_subscriber(SdoSubscriber& subscriber) { _subscriber_list.push_back(&subscriber); }
    void unregister_subscriber(SdoSubscriber& subscriber) { _subscriber_list.remove(&subscriber); }
protected:
    std::list<SdoSubscriber*> _subscriber_list;
};


class SdoSubscriber {
private:
    SdoPublisher& _publisher;
public:
    SdoSubscriber(SdoPublisher& publisher)
            : _publisher(publisher) {
        _publisher.register_subscriber(*this);
    }
    virtual ~SdoSubscriber() {
        _publisher.unregister_subscriber(*this);
    }
    virtual FrameHandlingStatus handle_sdo(const SdoAnswer& answer) = 0;
    void unsubscribe() { _publisher.unregister_subscriber(*this); }
};


// Expedited SDO by the description of the dictionary: a request names an
// object by its key or its path, and the description says whether it may be
// read or written, how many bytes its value takes and which word a command
// asks for.
class ServerSdoService : public impl::FrameReceiver, public SdoPublisher {
private:
    impl::Server& _server;
    canid_t _id;

    // Objects asked for quietly and not answered yet: neither the request nor
    // the answer is logged. A bulk transfer asks this way, so that it takes a
    // line in the log rather than two for each parameter. An abort is logged
    // all the same, and a request made aloud for the object lifts the mark.
    std::set<od::Key> _quiet_requests;
    std::mutex _quiet_mtx;
public:
    ServerSdoService(impl::Server& server);
    void update_node_id();

    virtual FrameHandlingStatus handle_frame(const can_frame& frame) override;

    ODAccessStatus read(od::Key key, bool quiet = false);
    ODAccessStatus write(od::Key key, ExpeditedSdoData data, bool quiet = false);
    // A command: a write of the word its signature names, or of 0.
    ODAccessStatus command(od::Key key, bool quiet = false);

    ODAccessStatus read(std::string_view path, bool quiet = false);
    ODAccessStatus write(std::string_view path, ExpeditedSdoData data, bool quiet = false);
    ODAccessStatus command(std::string_view path, bool quiet = false);
private:
    std::optional<od::Item> _resolve(od::Key key) const;
    std::optional<od::Key> _find(std::string_view path) const;
    void _send(const od::Item& item, uint32_t cs, std::optional<ExpeditedSdoData> data);
    FrameHandlingStatus _handle(const can_frame& frame, SdoType type);
    void _mark_request(od::Key key, bool quiet);
    bool _take_quiet_request(od::Key key);
};


} // namespace ucanopen
