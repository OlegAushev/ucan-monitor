#include "impl_server.h"


namespace ucanopen {


impl::Server::Server(std::shared_ptr<can::Socket> socket, NodeId node_id, const std::string& name,
                     std::shared_ptr<const od::Description> description)
        : _name(name)
        , _node_id(node_id)
        , _socket(socket)
        , _description(std::move(description)) {}


} // namespace ucanopen
