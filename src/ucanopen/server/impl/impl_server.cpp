#include "impl_server.h"


namespace ucanopen {


impl::Server::Server(std::shared_ptr<can::Socket> socket, NodeId node_id, const std::string& name, const ObjectDictionary& dictionary)
        : _name(name)
        , _node_id(node_id)
        , _socket(socket)
        , _dictionary(dictionary) {
    for (const auto& [key, object] : _dictionary.entries) {
        // create aux dictionary for faster search by {category, subcategory, name}
        _dictionary_aux.insert({
                {object.category, object.subcategory, object.name},
                _dictionary.entries.find(key)});
    }
}


ODAccessStatus impl::Server::find_od_entry_to_read(std::string_view category,
                                                   std::string_view subcategory,
                                                   std::string_view name,
                                                   ODEntryIter& ret_entry) const {
    ret_entry = find_od_entry(category, subcategory, name);
    if (ret_entry == _dictionary.entries.end()) {
        bsclog::error("Cannot read {}::{}::{}::{}: no such OD entry.", _name, category, subcategory, name);
        return ODAccessStatus::not_found;
    } else if (ret_entry->second.has_read_permission() == false) {
        bsclog::error("Cannot read {}::{}::{}::{}: access denied.", _name, category, subcategory, name);
        return ODAccessStatus::access_denied;
    }
    return ODAccessStatus::success;
}


ODAccessStatus impl::Server::find_od_entry_to_write(std::string_view category,
                                                    std::string_view subcategory,
                                                    std::string_view name,
                                                    ODEntryIter& ret_entry) const {
    ret_entry = find_od_entry(category, subcategory, name);
    if (ret_entry == _dictionary.entries.end()) {
        bsclog::error("Cannot write {}::{}::{}::{}: no such OD entry.", _name, category, subcategory, name);
        return ODAccessStatus::not_found;;
    } else if (ret_entry->second.has_write_permission() == false) {
        bsclog::error("Cannot write {}::{}::{}::{}: access denied.", _name, category, subcategory, name);
        return ODAccessStatus::access_denied;
    }
    return ODAccessStatus::success;
}


ODAccessStatus impl::Server::find_od_entry_to_exec(std::string_view category,
                                                   std::string_view subcategory,
                                                   std::string_view name,
                                                   ODEntryIter& ret_entry) const {
    ret_entry = find_od_entry(category, subcategory, name);
    if (ret_entry == _dictionary.entries.end()) {
        bsclog::error("Cannot execute {}::{}::{}::{}: no such OD entry.", _name, category, subcategory, name);
        return ODAccessStatus::not_found;
    } else if ((ret_entry->second.data_type != ODObjectDataType::OD_EXEC) || (ret_entry->second.has_write_permission() == false)) {
        bsclog::error("Cannot execute {}::{}::{}::{}: not executable OD entry.", _name, category, subcategory, name);
        return ODAccessStatus::access_denied;
    }
    return ODAccessStatus::success;
}


} // namespace ucanopen
