/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <rohit/base/error.h>
#include <rohit/base/maths.h>
#include <rohit/net/ipv6addr.h>
#include <rohit/log/log.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <filesystem>

namespace MMS {

constexpr ipv6_socket_addr_t::operator sockaddr_in6() const {
    sockaddr_in6 sockaddr = {};
    sockaddr.sin6_family = AF_INET6;
    sockaddr.sin6_addr.__in6_u.__u6_addr32[0] = addr.addr_32[0];
    sockaddr.sin6_addr.__in6_u.__u6_addr32[1] = addr.addr_32[1];
    sockaddr.sin6_addr.__in6_u.__u6_addr32[2] = addr.addr_32[2];
    sockaddr.sin6_addr.__in6_u.__u6_addr32[3] = addr.addr_32[3];
    sockaddr.sin6_port = port.get_network_port();
    return sockaddr;
}

inline int create_socket() {
    int socket_id = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (socket_id < 0) {
        log<log_t::SOCKET_CREATE_FAILED>(errno);
        throw exception_t(MMS::error_helper_t::socket_create_ret());
    }

    log<log_t::SOCKET_CREATE_SUCCESS>(socket_id);
    return socket_id;
}

class socket_t {
protected:
    int socket_id;

    inline socket_t() : socket_id(socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) {
        if (socket_id < 0) {
        log<log_t::SOCKET_CREATE_FAILED>(errno);
            throw exception_t(MMS::error_helper_t::socket_create_ret());
        }

        log<log_t::SOCKET_CREATE_SUCCESS>(socket_id);
    }

public:
    constexpr socket_t(const int socket_id) : socket_id(socket_id) {}
    constexpr socket_t(socket_t &&sock) : socket_id(sock.socket_id) { sock.socket_id = 0; }
    ~socket_t() {
        if (socket_id) {
            ::close(socket_id);
            socket_id = 0;
        }
    }

    inline err_t close() {
        if (socket_id) {
            auto ret = ::close(socket_id);
            if (ret == -1) {
                log<log_t::SOCKET_CLOSE_FAILED>(socket_id, errno);
                return err_t::CLOSE_FAILURE;
            } else {
                log<log_t::SOCKET_CLOSE_SUCCESS>(socket_id);
            }
            socket_id = 0;
        }
        return err_t::SUCCESS;
    }

    inline err_t read(void *buf, const size_t buf_size, size_t &read_len) const {
        int ret = ::recv(socket_id, buf, buf_size, MSG_DONTWAIT);
        switch(ret) {
        case 0:
            read_len = 0;
            return err_t::ORDERLY_SHUTDOWN;
        
        case -1:
            read_len = 0;
            switch (errno) {
            case EAGAIN: // This will be called if no data is available from peer
            // case EWOULDBLOCK: EWOULDBLOCK == EAGAIN
                return err_t::SOCKET_RETRY;

            case ECONNREFUSED:
            case ENOTCONN:
                return err_t::CONNECTION_REFUSED;

            case EFAULT:
            case ENOMEM:
                throw exception_t(err_t::CRITICAL_FAILURE);

            case EBADF:
                return err_t::BAD_FILE_DESCRIPTOR;

            // case EINTR:
            // case EINVAL:
            default:
                return err_t::RECEIVE_FAILURE;
            }

        default:
            read_len = ret;
            return err_t::SUCCESS;
        }
    }

    inline err_t write_wait(
                const void *buf,
                const size_t send_len,
                size_t &actual_sent,
                int64_t wait_in_millisecond = config::attempt_to_write_wait_in_ms,
                int write_attempt = config::attempt_to_write) const
    {
        int attempt_to_write = 0;
        while(attempt_to_write < write_attempt) {
            int ret = ::write(socket_id, buf, send_len);
            if (ret > 0) {
                actual_sent = ret;
                break;
            }

            if (ret < 0) {
                return err_t::SEND_FAILURE;
            }
            
            ++attempt_to_write;
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_in_millisecond));
        }

        if (attempt_to_write == write_attempt) {
            return err_t::SOCKET_WRITE_ZERO;
        }

        return err_t::SUCCESS;
    }

    inline err_t write(const void *buf, const size_t send_len, size_t &actual_sent) const {
        int ret = ::send(socket_id, buf, send_len, MSG_DONTWAIT );
        if (ret == -1) {
            switch(ret) {
            case EAGAIN:
            case EALREADY:
            case ENOBUFS:
            // case EWOULDBLOCK: EWOULDBLOCK == EAGAIN
                return err_t::SOCKET_RETRY;

            case ECONNREFUSED:
            case ENOTCONN:
            case EPIPE:
                return err_t::CONNECTION_REFUSED;

            case EFAULT:
            case ENOMEM:
                throw exception_t(err_t::CRITICAL_FAILURE);

            case EBADF:
                return err_t::BAD_FILE_DESCRIPTOR;

            // case EINTR:
            // case EINVAL:
            default:
                return err_t::SEND_FAILURE;
            }
        }
        actual_sent = ret;
        if (actual_sent < send_len) {
            return err_t::SOCKET_RETRY;
        }
        return err_t::SUCCESS;
    }

    constexpr err_t accept() {
        return err_t::SUCCESS;
    }

    inline const ipv6_socket_addr_t get_peer_ipv6_addr() const {
        sockaddr_in6 addr;
        socklen_t len = sizeof(addr);
        getpeername(socket_id, reinterpret_cast<struct sockaddr *>(&addr), &len);

        ipv6_port_t &port = *reinterpret_cast<ipv6_port_t *>(&addr.sin6_port);
        return ipv6_socket_addr_t(&addr.sin6_addr.__in6_u, port);
    }

    inline const ipv6_socket_addr_t get_local_ipv6_addr() const {
        sockaddr_in6 addr;
        socklen_t len = sizeof(addr);
        getsockname(socket_id, (struct sockaddr *)&addr, &len);

        ipv6_port_t &port = *reinterpret_cast<ipv6_port_t *>(&addr.sin6_port);
        return ipv6_socket_addr_t(&addr.sin6_addr.__in6_u, port);
    }

    // Returns local or socket IP address
    inline operator const ipv6_socket_addr_t() const {
        return get_local_ipv6_addr();
    }

    inline bool is_null() const { return socket_id == 0; }

    inline bool set_non_blocking() {
        int flags = fcntl(socket_id, F_GETFL, 0);
        if (flags != -1) {
            flags |= O_NONBLOCK;
            flags = fcntl(socket_id, F_SETFL, flags);
        }

        return flags != -1;
    }

    constexpr auto GetFD() const { return socket_id; }

    inline bool is_closed() const { return socket_id == 0; }
};

inline std::ostream& operator<<(std::ostream& os, const socket_t &client_id) {
    return os << client_id.get_local_ipv6_addr();
}

class server_socket_t : public socket_t {
public:
    inline server_socket_t(const int port) {
        int enable = 1;
        if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&enable,sizeof(enable)) < 0) {
            close(); 
            throw exception_t(error_helper_t::sockopt_ret());
        }

        struct sockaddr_in6 addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(port);
        addr.sin6_addr = in6addr_any;

        if (bind(socket_id, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close();
            throw exception_t(err_t::BIND_FAILURE);
        }
        log<log_t::SOCKET_BIND_SUCCESS>(socket_id, port);

        if (listen(socket_id, config::socket_backlog) < 0) {
            close();
            throw exception_t(err_t::LISTEN_FAILURE);
        }
        log<log_t::SOCKET_LISTEN_SUCCESS>(socket_id, port);
    }

    inline socket_t accept() {
        auto client_id = ::accept4(socket_id, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (client_id == -1) {
            if (errno == EAGAIN) {
                return 0;
            }
            throw exception_t(err_t::ACCEPT_FAILURE);
        }

        log<log_t::SOCKET_ACCEPT_SUCCESS>(socket_id, client_id);
        return client_id;
    }

};

class client_socket_t : public socket_t {
private:
    inline err_t connect(const ipv6_socket_addr_t &ipv6addr) {
        sockaddr_in6 in6_addr = ipv6addr;
        if (::connect(socket_id, (struct sockaddr*)&in6_addr, sizeof(in6_addr)) == 0)
            return err_t::SUCCESS;
        return error_helper_t::socket_connect_ret();
    }

public:
    using socket_t::socket_t;
    client_socket_t(const ipv6_socket_addr_t &ipv6addr) {
        err_t err = connect(ipv6addr);
        if (isFailure(err)) throw exception_t(err);
    }
};


} // namespace MMS