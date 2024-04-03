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
#include <vector>

#include <bsclog/bsclog.h>
#include <cmdexec/cmdexec.h>


namespace can {


enum class Status {
    ok,
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


const std::vector<std::string> interface_list = {"can0", "can1"};
const std::vector<std::string> bitrate_list = {"125000", "250000", "500000", "1000000"};
const std::set<std::filesystem::path> scripts_location_list = {"", "scripts", "..", "../scripts"};


} // namespace detail


class Socket {
private:
    int _socket{-1};
    ifreq _ifr;
    sockaddr_can _addr;

    pollfd _recv_fd;
    static constexpr std::chrono::milliseconds _recv_timeout{1};

    std::mutex _send_mtx;
    std::mutex _recv_mtx;

public:
    Socket();
    ~Socket();
    Socket(const Socket& other) = delete;
    Socket& operator=(const Socket& other) = delete;

    Status connect(const std::string& interface, const std::string& bitrate);
    Status disconnect();

    Status send(const can_frame& frame);
    Status recv(can_frame& frame);
    
    bool good() const { return _socket >= 0; }
private:
    Status _create_socket(const std::string& interface);
    std::filesystem::path _find_script(std::filesystem::path name);

    std::string exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }
};


} // namespace can
