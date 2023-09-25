#pragma once


#include "../impl/impl_server.h"
#include <vector>
#include <map>


namespace ucanopen {

class ServerConfigService {
private:
    impl::Server& _server;
    std::map<std::string_view, std::vector<std::string_view>> _object_list;
public:
    ServerConfigService(impl::Server& server);
    std::map<std::string_view, std::vector<std::string_view>> object_list() const { return _object_list; }
};

} // namespace ucanopen

