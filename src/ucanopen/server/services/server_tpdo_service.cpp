#include "server_tpdo_service.h"


namespace ucanopen {

ServerTpdoService::ServerTpdoService(impl::Server& server)
        : _server(server)
{}


void ServerTpdoService::register_tpdo(CobTpdo tpdo, std::chrono::milliseconds timeout, std::function<void(const can_payload&)> handler) {
    canid_t id = calculate_cob_id(to_cob(tpdo), _server.node_id());
    _tpdo_msgs.emplace(tpdo, Message{id, timeout, std::chrono::steady_clock::now(), can_payload{}, handler});

    uint16_t index = 0x1A00 + std::to_underlying(tpdo);

    if (!_server.dictionary().entries.contains({index, 0x00})) {
        return;
    }

    uint16_t subindex = 0x01;
    uint64_t offset = 0;
    while (_server.dictionary().entries.contains({index, subindex})) {
        uint64_t mask = 0;
        const auto& entry = _server.dictionary().entries.at({index, subindex});

        for (size_t i = 0; i < od_object_data_type_sizes[entry.data_type]; ++i) {
            mask |= 0xFF << i * 8;
        }

        _tpdo_mapping[tpdo].push_back({entry.subcategory, entry.name, offset, mask});
        offset += od_object_data_type_sizes[entry.data_type] * 8;

        ++subindex;
    }
}


void ServerTpdoService::update_node_id() {
    std::lock_guard<std::mutex> lock(_mtx);
    for (auto& [tpdo, message] : _tpdo_msgs) {
        message.id = calculate_cob_id(to_cob(tpdo), _server.node_id());
    }
}


FrameHandlingStatus ServerTpdoService::handle_frame(const can_frame& frame) {
    std::lock_guard<std::mutex> lock(_mtx);
    for (auto& [tpdo, message] : _tpdo_msgs) {
        if (frame.can_id != message.id) { continue; }

        message.timepoint = std::chrono::steady_clock::now();
        can_payload payload{};
        std::copy(frame.data, std::next(frame.data, frame.can_dlc), payload.begin());
        message.payload = payload;
        message.handler(payload);

        for (auto& subscriber : _subscriber_list) {
            subscriber->handle_tpdo(tpdo, payload);
        }

        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::id_mismatch;
}

} // namespace ucanopen

