/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/base/error.h>
#include <mms/base/maths.h>
#include <mms/net/ipv6addr.h>
#include <mms/log/log.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace MMS {

class tcp_socket_t {
protected:
    int socket_id;

    inline tcp_socket_t(bool nonblocking) : socket_id(socket(AF_INET6, SOCK_STREAM | SOCK_CLOEXEC | (nonblocking ? SOCK_NONBLOCK : 0), IPPROTO_TCP)) {
        if (socket_id < 0) {
        log<log_t::TCP_SOCKET_CREATE_FAILED>(errno);
            throw exception_t(MMS::error_helper_t::socket_create_ret());
        }

        log<log_t::TCP_SOCKET_CREATE_SUCCESS>(socket_id);
    }

public:
    constexpr tcp_socket_t(const int socket_id) : socket_id(socket_id)  {}
    constexpr tcp_socket_t(tcp_socket_t &&sock) : socket_id(sock.socket_id) { sock.socket_id = 0; }
    ~tcp_socket_t() {
        if (socket_id) {
            ::close(socket_id);
            socket_id = 0;
        }
    }

    inline err_t close() {
        if (socket_id) {
            auto ret = ::close(socket_id);
            if (ret == -1) {
                log<log_t::TCP_SOCKET_CLOSE_FAILED>(socket_id, errno);
                return err_t::CLOSE_FAILURE;
            } else {
                log<log_t::TCP_SOCKET_CLOSE_SUCCESS>(socket_id);
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
                int64_t wait_in_millisecond = 50,
                int write_attempt = 20) const
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

    constexpr auto GetFD() const { return socket_id; }

    inline bool is_closed() const { return socket_id == 0; }
};

class tcp_client_socket_t : public tcp_socket_t {
private:
    inline err_t connect(const ipv6_socket_addr_t &ipv6addr) {
        sockaddr_in6 in6_addr = ipv6addr;
        if (::connect(socket_id, (struct sockaddr*)&in6_addr, sizeof(in6_addr)) == 0)
            return err_t::SUCCESS;
        return error_helper_t::socket_connect_ret();
    }

public:
    using tcp_socket_t::tcp_socket_t;
    tcp_client_socket_t(const ipv6_socket_addr_t &ipv6addr, bool nonblocking) : tcp_socket_t { nonblocking } {
        err_t err = connect(ipv6addr);
        if (isFailure(err)) throw exception_t(err);
    }
};


} // namespace MMS