#pragma once


#include <ucanopen/server/impl/impl_server.h>
#include <ucanopen/server/services/sdo/server_sdo_service.h>
#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>


namespace ucanopen {


// One request of a sequence: a read, a write of `write_value` or a command,
// and how it went.
struct SdoStep {
    enum class Kind { read, write, command };
    enum class Status {
        pending,
        done,
        refused,   // the server aborted the request, see `abort_code`
        timed_out, // the server did not answer
        cancelled  // the sequence stopped before it got here
    };

    od::Key key{};
    Kind kind{Kind::read};
    std::optional<ExpeditedSdoData> write_value{};
    Status status{Status::pending};
    ExpeditedSdoData value{}; // what a read got back
    SdoAbortCode abort_code{SdoAbortCode::no_error};

    static SdoStep read(od::Key key) { return {.key = key}; }
    static SdoStep write(od::Key key, ExpeditedSdoData value) {
        return {.key = key, .kind = Kind::write, .write_value = value};
    }
    static SdoStep command(od::Key key) { return {.key = key, .kind = Kind::command}; }
};


// SDO requests that the CAN thread works off one at a time: send() asks,
// handle_sdo() takes the answer. One request in flight keeps the server's
// answers in order and the bus as quiet as a single object read by hand. A
// step that goes unanswered is asked again once; three steps in a row that
// get no answer stop the sequence, as the server is gone.
//
// What a step brings may call for more steps: a text first reads its length,
// then its characters. `next`, given to start(), runs on each step finished,
// in the CAN thread, and the steps it returns go at the end.
class SdoSequence : public SdoSubscriber {
public:
    // How the last sequence ended; `none` while one runs or before the first.
    enum class Outcome {
        none,
        completed,
        cancelled,
        no_response
    };
    using Next = std::function<std::vector<SdoStep>(const SdoStep&)>;
private:
    ServerSdoService& _sdo_service;

    mutable std::mutex _mtx;
    std::vector<SdoStep> _steps;
    Next _next;
    size_t _step_idx{0};
    bool _awaiting{false};
    unsigned _attempts{0};
    unsigned _unanswered{0}; // steps in a row that got no answer
    std::chrono::time_point<std::chrono::steady_clock> _request_timepoint;
    Outcome _outcome{Outcome::none};

    static constexpr std::chrono::milliseconds _timeout{500};
    static constexpr unsigned _max_attempts{2};
    static constexpr unsigned _max_unanswered{3};
public:
    explicit SdoSequence(ServerSdoService& sdo_service);

    // Starts working off `steps` in order. Refused while a sequence runs.
    bool start(std::vector<SdoStep> steps, Next next = {});
    void cancel();
    bool busy() const;
    std::pair<size_t, size_t> progress() const; // steps finished, steps in all
    Outcome outcome() const;
    std::vector<SdoStep> steps() const;

    void send();
    virtual FrameHandlingStatus handle_sdo(const SdoAnswer& answer) override;
private:
    void _request(std::chrono::time_point<std::chrono::steady_clock> now);
    void _advance();
    void _finish(Outcome outcome);
};


} // namespace ucanopen
