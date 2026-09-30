#pragma once


#include <ucanopen/server/impl/impl_server.h>
#include <mutex>
#include <set>


namespace ucanopen {


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
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) = 0;
    void unsubscribe() { _publisher.unregister_subscriber(*this); }
};


class ServerSdoService : public impl::FrameReceiver, public SdoPublisher {
private:
    impl::Server& _server;
    canid_t _id;

    // Objects asked for quietly and not answered yet: neither the request nor
    // the answer is logged. A bulk transfer asks this way, so that it takes a
    // line in the log rather than two for each parameter. An abort is logged
    // all the same, and a request made aloud for the object lifts the mark.
    std::set<ODObjectKey> _quiet_requests;
    std::mutex _quiet_mtx;
public:
    ServerSdoService(impl::Server& server);
    void update_node_id();
    const impl::Server server() const { return _server; }

    virtual FrameHandlingStatus handle_frame(const can_frame& frame) override;
    ODAccessStatus read(std::string_view category, std::string_view subcategory, std::string_view name, bool quiet = false);
    ODAccessStatus write(std::string_view category, std::string_view subcategory, std::string_view name, ExpeditedSdoData sdo_data, bool quiet = false);
    ODAccessStatus write(std::string_view category, std::string_view subcategory, std::string_view name, const std::string& value);
    ODAccessStatus exec(std::string_view category, std::string_view subcategory, std::string_view name);
    void restore_default_parameter(std::string_view category, std::string_view subcategory, std::string_view name);
private:
    FrameHandlingStatus _handle_read_expedited(const can_frame& frame);
    FrameHandlingStatus _handle_write_expedited(const can_frame& frame);
    FrameHandlingStatus _handle_abort(const can_frame& frame);
    void _mark_request(const ODObjectKey& key, bool quiet);
    bool _take_quiet_request(const ODObjectKey& key);
};

} // namespace ucanopen

