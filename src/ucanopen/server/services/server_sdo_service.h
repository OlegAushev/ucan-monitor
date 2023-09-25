#pragma once


#include "../impl/impl_server.h"
#include "server_watch_service.h"


namespace ucanopen {

class ServerSdoService : public impl::FrameReceiverInterface, public impl::SdoPublisher {
private:
    impl::Server& _server;
    canid_t _id;
public:
    ServerSdoService(impl::Server& server);
    void update_node_id();

    virtual FrameHandlingStatus handle_frame(const can_frame& frame) override;
private:
    FrameHandlingStatus _handle_read_expedited(const can_frame& frame);
    FrameHandlingStatus _handle_write_expedited(const can_frame& frame);
    FrameHandlingStatus _handle_abort(const can_frame& frame);
};

} // namespace ucanopen

