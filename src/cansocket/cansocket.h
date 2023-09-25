#pragma once


#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <poll.h>

#include <chrono>
#include <filesystem>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <utility>

#include <log/log.h>


namespace can {

enum class Error {
    none,
    invalid_argument,
    script_not_found,
    device_not_found,
    socketcan_init_failed,
    script_exec_failed,
    socket_creation_failed,
    socket_closing_failed,
    socket_binding_failed,
    socket_closed,
    send_error,
    recv_timeout,
    recv_error,
    interface_retrieving_failed,
};


namespace detail {

const std::set<std::string> interface_list = {"can0", "can1"};
const std::set<int> bitrate_list = {125'000, 250'000, 500'000, 1'000'000};
const std::set<std::filesystem::path> scripts_location_list = {
    "",
    "scripts",
    "..",
    "../scripts"
};

} // namespace detail


class Socket {
private:
    int _socket = -1;
    ifreq _ifr;
    sockaddr_can _addr;

    pollfd _recv_fd;
    static constexpr std::chrono::milliseconds _recv_timeout = std::chrono::milliseconds(1);

    std::mutex _send_mtx;
    std::mutex _recv_mtx;

public:
    Socket();
    ~Socket();
    Socket(const Socket& other) = delete;
    Socket& operator=(const Socket& other) = delete;

    Error connect(const std::string& interface, int bitrate);
    Error disconnect();

    Error send(const can_frame& frame);
    Error recv(can_frame& frame);

private:
    Error _create_socket(const std::string& interface);
    std::filesystem::path _find_script(std::filesystem::path name);
};

} // namespace can

