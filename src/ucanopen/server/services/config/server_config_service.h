#pragma once


#include <ucanopen/server/impl/impl_server.h>
#include <ucanopen/server/services/sdo/server_sdo_service.h>
#include <chrono>
#include <map>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>


namespace ucanopen {


// One parameter of a bulk transfer: a read, or a write of `write_value`, and
// how it went.
struct ConfigStep {
    enum class Status {
        pending,
        done,
        refused,    // the server aborted the request, see `abort_code`
        timed_out,  // the server did not answer
        cancelled   // the transfer stopped before it got here
    };

    ODEntryIter entry{};
    std::optional<ExpeditedSdoData> write_value{};
    Status status{Status::pending};
    ExpeditedSdoData value{};  // what a read got back
    SdoAbortCode abort_code{SdoAbortCode::no_error};
};


class ServerConfigService : public SdoSubscriber {
public:
    // How the last transfer ended; `none` while one runs or before the first.
    enum class Outcome {
        none,
        completed,
        cancelled,
        no_response  // several parameters in a row went unanswered
    };
private:
    impl::Server& _server;
    ServerSdoService& _sdo_service;
    std::map<std::string_view, std::vector<const ODObject*>> _objects;
    std::vector<ODEntryIter> _entries;

    // The bulk transfer. The UI thread starts it and watches it, the CAN
    // thread works it off one request at a time: send() asks, handle_sdo()
    // takes the answer. One request in flight keeps the server's answers in
    // order and the bus as quiet as a single parameter being set by hand.
    mutable std::mutex _mtx;
    std::vector<ConfigStep> _steps;
    size_t _step_idx{0};
    bool _awaiting{false};
    unsigned _attempts{0};
    unsigned _unanswered{0}; // parameters in a row that got no answer
    std::chrono::time_point<std::chrono::steady_clock> _request_timepoint;
    Outcome _outcome{Outcome::none};

    static constexpr std::chrono::milliseconds _timeout{500};
    static constexpr unsigned _max_attempts{2};
    static constexpr unsigned _max_unanswered{3};
public:
    ServerConfigService(impl::Server& server, ServerSdoService& sdo_service);
    const std::map<std::string_view, std::vector<const ODObject*>>& objects() const { return _objects; }

    // Every config object, in dictionary order.
    const std::vector<ODEntryIter>& entries() const { return _entries; }

    // Starts working off `steps` in order. Refused while a transfer runs.
    bool start(std::vector<ConfigStep> steps);
    void cancel();
    bool busy() const;
    std::pair<size_t, size_t> progress() const; // steps finished, steps in all
    Outcome outcome() const;
    std::vector<ConfigStep> steps() const;

    void send();
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
private:
    void _request(std::chrono::time_point<std::chrono::steady_clock> now);
    void _advance();
    void _finish(Outcome outcome);
};


} // namespace ucanopen
