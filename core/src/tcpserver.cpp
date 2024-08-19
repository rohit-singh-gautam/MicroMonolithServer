/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/net/tcpserver.h>

namespace MMS::net::tcp {

thread_local buffer_t connection_t::tempbuffer { };

err_t connection_t::ProcessRead() {
    size_t offset { 0 };
    while(true) {
        auto [buffer, buffer_size] = tempbuffer.GetBuffer(offset);
        auto ret = ::recv(GetFD(), buffer, buffer_size, MSG_DONTWAIT);
        switch(ret) {
        case 0:
            return err_t::ORDERLY_SHUTDOWN;
            break;

        case -1:
            switch(errno) {
            case EAGAIN: // This will be called if no data is available from peer
            // case EWOULDBLOCK: EWOULDBLOCK == EAGAIN
                goto EXIT_LOOP;
            
            case EFAULT:
            case ENOMEM:
                throw exception_t(err_t::CRITICAL_FAILURE);

            // case ECONNREFUSED:
            // case ENOTCONN:
            // case EBADF:
            // case EINTR:
            // case EINVAL:
            default: 
                log<log_t::TCP_SERVER_PEER_READ_FAILED>(GetFD(), errno);
                // Close will take care of termination
                Close();
                return err_t::BAD_FILE_DESCRIPTOR;
            }
            break;

        default:
            offset += ret;
            break;
        }
    }

EXIT_LOOP:
    if (offset) {
        protocol_implementation->ProcessRead(tempbuffer.GetBuffer<uint8_t *>(), offset, *this);
        log<log_t::TCP_CONNECTION_READ>(GetFD(), offset);
    }
    else log<log_t::TCP_CONNECTION_EMPTY_READ>(GetFD());

    return pending_wirte.empty() ? err_t::SUCCESS : err_t::SOCKET_RETRY;
}

err_t connection_t::ProcessWrite() {
    while(!pending_wirte.empty()) {
        auto &currentbuffer = pending_wirte.front();
        while(!currentbuffer.Completed()) {
            auto [buffer, size] = currentbuffer.GetBuffer<void *>();
            auto ret = ::send(GetFD(), buffer, size, MSG_DONTWAIT );
            if (ret <= -1) {
                switch(ret) {
                case EAGAIN:
                case EALREADY:
                case ENOBUFS:
                // case EWOULDBLOCK: EWOULDBLOCK == EAGAIN
                    return err_t::SOCKET_RETRY;

                case EFAULT:
                case ENOMEM:
                    throw exception_t(err_t::CRITICAL_FAILURE);

                // case ECONNREFUSED:
                // case ENOTCONN:
                // case EPIPE:
                // case EBADF:
                // case EINTR:
                // case EINVAL:
                default:
                    // Close will take care of termination
                    log<log_t::TCP_SERVER_PEER_WRITE_FAILED>(GetFD(), errno);
                    Close();
                    return err_t::BAD_FILE_DESCRIPTOR;
                }
            }
            currentbuffer.AddOffset(static_cast<size_t>(ret));
            if (static_cast<size_t>(ret) < size) return err_t::SOCKET_RETRY;
        }

        if (!currentbuffer.Completed()) break;
        pending_wirte.pop();
    }
    return err_t::SUCCESS;
}

void connection_t::WriteNoCopy(uint8_t* buffer, size_t bytesize, size_t byteoffset) {
    pending_wirte.emplace(buffer, bytesize, byteoffset);
}

int server_t::CreateServerSocket(int port) {
    const int socket_id = socket(AF_INET6, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_TCP);
    int enable = 1;
    if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&enable,sizeof(enable)) < 0) {
        ::close(socket_id); 
        throw setsockopt_fail_t(error_helper_t::sockopt_ret());
    }

    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    addr.sin6_addr = in6addr_any;

    if (bind(socket_id, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(socket_id);
        throw bind_fail_t { };
    }
    log<log_t::TCP_SOCKET_BIND_SUCCESS>(socket_id, port);

    if (listen(socket_id, socket_backlog) < 0) {
        ::close(socket_id);
        throw listen_fail_t { };
    }
    log<log_t::TCP_SOCKET_LISTEN_SUCCESS>(socket_id, port);

    return socket_id;
}

err_t server_t::ProcessRead() {
    log<log_t::TCP_SERVER_RECEIVED_EVENT>(GetFD());
    try {
        auto peer_id = ::accept4(GetFD(), NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (peer_id == -1) {
            if (errno == EAGAIN) {
                return err_t::SUCCESS;
            }
            throw accept_fail_t { };
        }
        log<log_t::TCP_SOCKET_ACCEPT_SUCCESS>(GetFD(), peer_id);

        auto protocol = protocol_creator.create_protocol();
        auto connection = new connection_t(peer_id, protocol);
        auto ret = listener->add(connection);
        if (ret == err_t::SUCCESS) {
            log<log_t::TCP_SERVER_PEER_CREATED>(GetFD(), connection->GetFD(), connection->get_peer_ipv6_addr());
        } else {
            log<log_t::TCP_SERVER_PEER_CREATED>(GetFD(), connection->GetFD(), connection->get_peer_ipv6_addr());
            delete connection;
        }
    } catch (const exception_t e) {
        if (e == err_t::ACCEPT_FAILURE) {
            log<log_t::TCP_SERVER_ACCEPT_FAILED>(GetFD(), errno);
        }
    }
    return err_t::SUCCESS;
}

} // namespace MMS::net::tcp