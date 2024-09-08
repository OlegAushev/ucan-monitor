#pragma once


#include <ucanopen/server/impl/impl_server.h>
#include <vector>
#include <map>


namespace ucanopen {


class ServerConfigService {
private:
    impl::Server& _server;
    std::map<std::string_view, std::vector<const ODObject*>> _objects;
public:
    ServerConfigService(impl::Server& server);
    const std::map<std::string_view, std::vector<const ODObject*>>& objects() const { return _objects; }
};


} // namespace ucanopen
