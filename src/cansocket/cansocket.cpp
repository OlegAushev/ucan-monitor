#include "cansocket.h"


namespace can {

Socket::Socket() {
    // check can0: may be interface is already enabled
    /* FIND SCRIPT */
    bsclog::info("Searching for SocketCAN checking script...");

    std::filesystem::path script_path = _find_script("socketcan_check.sh");
    if (script_path.empty()) {
        bsclog::warning("Failed to find SocketCAN checking script.");
        return;
    }

    bsclog::success("Found SocketCAN checking script: {}", script_path.string());

    /* RUN SCRIPT */
    std::string cmd = "sh " + script_path.string() + " " + "can0" + " 2>&1";
    bsclog::info("Checking SocketCAN interface can0, executing system command: {}", cmd);
    
    auto script_result = cmdexec::run(cmd);
    std::stringstream log_sstr(script_result.output);
    std::string log_str;
    while (std::getline(log_sstr, log_str, '\n')) {
        bsclog::log("{}", log_str);
    }

    if (script_result.exitcode == 0) {
        if (_create_socket("can0") != Error::none) {
            _socket = -1;
        }
    }
}


Socket::~Socket() {
    disconnect();
}


Error Socket::_create_socket(const std::string& interface) {
    /* CREATE SOCKET */
    bsclog::info("Creating CAN socket...");
    _socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (_socket < 0) {
        bsclog::error("Failed to create CAN socket.");
        return Error::socket_creation_failed;
    }

    std::strcpy(_ifr.ifr_name, interface.c_str());
    if (ioctl(_socket, SIOCGIFINDEX, &_ifr) < 0) {
        bsclog::error("Failed to retrieve CAN interface.");
        return Error::interface_retrieving_failed;
    }

    memset(&_addr, 0, sizeof(_addr));
    _addr.can_family = AF_CAN;
    _addr.can_ifindex = _ifr.ifr_ifindex;

    can_filter filter[1];
    filter[0].can_id = 0;
    filter[0].can_mask = 0x000;
    setsockopt(_socket, SOL_CAN_RAW, CAN_RAW_FILTER, filter, sizeof(can_filter));

    if (bind(_socket, (sockaddr*)&_addr, sizeof(_addr)) < 0) {
        bsclog::error("Failed to bind CAN socket.");
        return Error::socket_binding_failed;
    }

    _recv_fd.fd = _socket;
    _recv_fd.events = POLLIN;

    bsclog::success("Created CAN socket.");
    return Error::none;
}


Error Socket::connect(const std::string& interface, const std::string& bitrate) {
    _socket = -1;

    if (std::find(detail::interface_list.begin(), detail::interface_list.end(), interface) == detail::interface_list.end()
            || std::find(detail::bitrate_list.begin(), detail::bitrate_list.end(), bitrate) == detail::bitrate_list.end()) {
        return Error::invalid_argument;
    }

    std::lock_guard<std::mutex> lock1(_send_mtx);
    std::lock_guard<std::mutex> lock2(_recv_mtx);

    /* FIND SCRIPT */
    bsclog::info("Searching for SocketCAN enabling script...");
    
    std::filesystem::path script_path = _find_script("socketcan_enable.sh");
    if (script_path.empty()) {
        bsclog::error("Failed to find SocketCAN enabling script.");
        return Error::script_not_found;
    }

    bsclog::success("Found SocketCAN enabling script: {}", script_path.string());

    /* RUN SCRIPT */
    std::string cmd = "pkexec sh " + script_path.string() + " " + interface + " " + bitrate + " 2>&1";
    bsclog::info("Enabling SocketCAN interface {}, executing system command: {}", interface, cmd);

    auto pkexec_result = cmdexec::run(cmd);
    std::stringstream log_sstr(pkexec_result.output);
    std::string log_str;
    while (std::getline(log_sstr, log_str, '\n')) {
        bsclog::log("{}", log_str);
    }

    Error error;
    switch (pkexec_result.exitcode) {
    case 0:
        error = Error::none;
        break;
    case 1:
        error = Error::invalid_argument;
        break;
    case 2:
        error = Error::device_not_found;
        break;
    case 3:
        error = Error::socketcan_init_failed;
        break;
    default:
        error = Error::script_exec_failed;
        break;
    }

    if (error != Error::none) {
        bsclog::error("Failed to enable SocketCAN interface. Error code: {}", std::to_underlying(error));
        return error;
    }

    return _create_socket(interface);
}


Error Socket::disconnect() {
    if (_socket < 0) {
        bsclog::error("Failed to close CAN socket: no socket.");
        return Error::socket_closed;
    }

    std::lock_guard<std::mutex> lock1(_send_mtx);
    std::lock_guard<std::mutex> lock2(_recv_mtx);

    if (close(_socket) < 0) {
        bsclog::error("Failed to close CAN socket.");
        return Error::socket_closing_failed;
    } else {
        bsclog::success("Closed socket.");
        _socket = -1;
        return Error::none;
    }
}


std::filesystem::path Socket::_find_script(std::filesystem::path name) {
    std::filesystem::path script_path;
    for (auto loc : can::detail::scripts_location_list) {
        auto absolute_path = std::filesystem::absolute(loc/name);
        if (std::filesystem::exists(absolute_path)) {
            script_path = std::filesystem::canonical(loc/name);
        }
    }
    return script_path;
}


Error Socket::send(const can_frame& frame) {
    if (_socket < 0) {
        return Error::socket_closed;
    }

    std::lock_guard<std::mutex> lock(_send_mtx);

    if (::write(_socket, &frame, sizeof(can_frame)) != sizeof(can_frame)) {
        return Error::send_error;
    }
    return Error::none;
}


Error Socket::recv(can_frame& frame) {
    if (_socket < 0) {
        return Error::socket_closed;
    }

    int byte_count;

    std::lock_guard<std::mutex> lock(_recv_mtx);

    int ret = poll(&_recv_fd, 1, _recv_timeout.count());
    switch (ret) {
    case -1:
        return Error::recv_error;
        break;
    case 0:
        return Error::recv_timeout;
        break;
    default:
        byte_count = ::read(_socket, &frame, sizeof(can_frame));
        if (byte_count < 0) {
            return Error::recv_error;
        } else {
            return Error::none;
        }
        break;
    }
}

} // namespace can

