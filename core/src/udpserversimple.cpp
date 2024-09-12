/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/net/udpserversimple.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace MMS::net::udp {
void server_t::WriteNoCopy(FixedBuffer &&buffer) {

    pending_wirte.emplace(*current_client_addr, std::move(buffer));
}

err_t server_t::ProcessRead() {
    while(true) {
        sockaddr_in6 client_addr;
        client_addr.sin6_family = AF_INET6;
        socklen_t client_len = sizeof(client_addr);

        auto [buffer, buffer_size] = readbuffer.GetRawCurrentBuffer();
        ssize_t ret = recvfrom(GetFD(), buffer, buffer_size, 0, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

        switch(ret) {
        case 0:
            return err_t::ORDERLY_SHUTDOWN;

        case -1:
            switch(errno) {
            case EAGAIN: // This will be called if no data is available from peer
            // case EWOULDBLOCK: EWOULDBLOCK == EAGAIN
                return err_t::SUCCESS;
            
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
            readbuffer += ret;
            break;
        }
        if (readbuffer.index()) {
            current_client_addr = &client_addr;
            protocol_implementation->ProcessRead(make_const_stream(readbuffer.begin(), readbuffer.curr()));
            current_client_addr = nullptr;
            log<log_t::UDP_CONNECTION_READ>(GetFD(), readbuffer.index());
        }
        else log<log_t::UDP_CONNECTION_EMPTY_READ>(GetFD());
    }

    return err_t::SUCCESS;
}

err_t server_t::ProcessWrite() {
    while(!pending_wirte.empty()) {
        auto &[saddr, currentbuffer] = pending_wirte.front();
        auto ret = ::sendto(GetFD(), currentbuffer.begin(), currentbuffer.size(), MSG_NOSIGNAL, reinterpret_cast<sockaddr*>(&saddr), sizeof(sockaddr_in6) );
        if (ret <= -1) {
            switch(errno) {
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
        if (static_cast<size_t>(ret) < currentbuffer.size()) return err_t::SOCKET_RETRY;

        pending_wirte.pop();
    }
    return err_t::SUCCESS;
}


} // namespace MMS::net::udp