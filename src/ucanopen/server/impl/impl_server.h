#pragma once


#include "../../ucanopen_def.h" 
#include <cansocket/cansocket.h>
#include <bsclog/bsclog.h>
#include <list>


namespace ucanopen {


enum class ODAccessStatus {
    success,
    not_found,
    access_denied,
    invalid_value
};


enum class FrameHandlingStatus {
    success,
    id_mismatch,
    invalid_format,
    object_not_found,
    irrelevant_frame
};


enum class SetupStatus {
    success,
    invalid_argument
};


class ServerHeartbeatService;
class ServerTpdoService;
class ServerRpdoService;
class ServerSdoService;
class TpdoSubscriber;


namespace impl {


class Server {
    friend class ucanopen::ServerHeartbeatService;
    friend class ucanopen::ServerTpdoService;
    friend class ucanopen::ServerRpdoService;
    friend class ucanopen::ServerSdoService;
protected:
    std::string _name{"unnamed"};
    NodeId _node_id;
    std::shared_ptr<can::Socket> _socket;

    const ObjectDictionary& _dictionary;
    ObjectDictionaryAux _dictionary_aux;

    NmtState _nmt_state{NmtState::stopped};
public:
    Server(std::shared_ptr<can::Socket> socket, NodeId node_id, const std::string& name, const ObjectDictionary& dictionary);
    
    std::string name() const { return _name; }
    NodeId node_id() const { return _node_id; }
    NmtState nmt_state() const { return _nmt_state; }
    const ObjectDictionary& dictionary() const { return _dictionary; }
public:
    ODEntryIter find_od_entry(std::string_view category,
                              std::string_view subcategory,
                              std::string_view name) const {
        auto iter = _dictionary_aux.find({category, subcategory, name});
        if (iter == _dictionary_aux.end()) {
            return _dictionary.entries.end();
        }
        return iter->second;
    }

    ODAccessStatus find_od_entry_to_read(std::string_view category,
                                         std::string_view subcategory,
                                         std::string_view name,
                                         ODEntryIter& ret_entry) const;
    ODAccessStatus find_od_entry_to_write(std::string_view category,
                                          std::string_view subcategory,
                                          std::string_view name,
                                          ODEntryIter& ret_entry) const;
    ODAccessStatus find_od_entry_to_exec(std::string_view category,
                                         std::string_view subcategory,
                                         std::string_view name,
                                         ODEntryIter& ret_entry) const;
};


class FrameReceiverInterface {
public:
    virtual FrameHandlingStatus handle_frame(const can_frame& frame) = 0;
};


class TpdoPublisher {
public:
    virtual ~TpdoPublisher() = default;
    void register_subscriber(TpdoSubscriber& subscriber) { _subscriber_list.push_back(&subscriber); }
    void unregister_subscriber(TpdoSubscriber& subscriber) { _subscriber_list.remove(&subscriber); }
protected:
    std::list<TpdoSubscriber*> _subscriber_list;
};


} // namespace impl


class TpdoSubscriber {
private:
    impl::TpdoPublisher& _publisher;
public:
    TpdoSubscriber(impl::TpdoPublisher& publisher)
            : _publisher(publisher) {
        _publisher.register_subscriber(*this);
    }
    virtual ~TpdoSubscriber() {
        _publisher.unregister_subscriber(*this);
    }
    virtual FrameHandlingStatus handle_tpdo(CobTpdo tpdo, const can_payload& payload) = 0;
    void unsubscribe() { _publisher.unregister_subscriber(*this); }
};


} // namespace ucanopen
