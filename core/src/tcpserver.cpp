//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2024  Rohit Jairaj Singh (rohit@singh.org.in)          //
//                                                                      //
// This program is free software: you can redistribute it and/or modify //
// it under the terms of the GNU General Public License as published by //
// the Free Software Foundation, either version 3 of the License, or    //
// (at your option) any later version.                                  //
//                                                                      //
// This program is distributed in the hope that it will be useful,      //
// but WITHOUT ANY WARRANTY; without even the implied warranty of       //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        //
// GNU General Public License for more details.                         //
//                                                                      //
// You should have received a copy of the GNU General Public License    //
// along with this program.  If not, see <https://www.gnu.org/licenses/>//
//////////////////////////////////////////////////////////////////////////

#include <mms/net/tcpserver.h>

namespace MMS::net::tcp {

err_t connection_t::ProcessRead() {
    while(true) {
        auto [buffer, buffer_size] = readbuffer.GetRawCurrentBuffer();
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
            readbuffer += ret;
            break;
        }
    }

EXIT_LOOP:
    if (readbuffer.index()) {
        protocol_implementation->ProcessRead(make_const_stream(readbuffer.begin(), readbuffer.curr()));
        log<log_t::TCP_CONNECTION_READ>(GetFD(), readbuffer.index());
    }
    else log<log_t::TCP_CONNECTION_EMPTY_READ>(GetFD());

    return err_t::SUCCESS;
}

err_t connection_t::ProcessWrite() {
    while(!pending_wirte.empty()) {
        auto &currentbuffer = pending_wirte.front();
        while(writeoffset != currentbuffer.size()) {
            const auto buffer = currentbuffer.begin() + writeoffset;
            size_t size = currentbuffer.size() - writeoffset;
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
            writeoffset += ret;
            if (static_cast<size_t>(ret) < size) return err_t::SOCKET_RETRY;
        }

        writeoffset = 0;
        pending_wirte.pop();
    }
    return err_t::SUCCESS;
}

void connection_base_t::WriteNoCopy(FixedBuffer &&buffer) {
    pending_wirte.emplace(std::move(buffer));
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

        auto protocol = protocol_creator.create_protocol(peer_id, { });
        auto connection = new connection_t(peer_id, protocol);
        protocol->SetProcessor(connection);
        log<log_t::HTTP_CREATED_PROTOCOL>(peer_id);
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