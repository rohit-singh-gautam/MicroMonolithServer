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
        size_t readlen  { 0 };
        auto [buffer, buffer_size] = tempbuffer.GetBuffer(offset);
        auto ret = connection_socket.read(buffer, buffer_size, readlen);
        offset += readlen;
        if (ret == err_t::SUCCESS) continue;
        if (ret == err_t::SOCKET_RETRY) break;
        if (ret == err_t::CONNECTION_REFUSED || ret == err_t::RECEIVE_FAILURE) {
            log<log_t::TCP_SERVER_PEER_READ_FAILED>(connection_socket.GetFD(), errno);
            // Close will take care of termination
            connection_socket.close();
            return err_t::BAD_FILE_DESCRIPTOR;
        }
        return ret;
    }
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
            size_t writtenlen { 0 };
            auto [buffer, size] = currentbuffer.GetBuffer<void *>();
            auto ret = connection_socket.write(buffer, size, writtenlen);
            currentbuffer.AddOffset(writtenlen);
            if (ret == err_t::SUCCESS) continue;
            if (ret == err_t::CONNECTION_REFUSED || ret == err_t::RECEIVE_FAILURE) {
                // Close will take care of termination
                log<log_t::TCP_SERVER_PEER_WRITE_FAILED>(connection_socket.GetFD(), errno);
                connection_socket.close();
                return err_t::BAD_FILE_DESCRIPTOR;
            }
            return ret;
        }

        if (!currentbuffer.Completed()) break;
        pending_wirte.pop();
    }
    return err_t::SUCCESS;
}

int connection_t::GetFD() const { 
    return connection_socket.GetFD();
}

void connection_t::WriteNoCopy(uint8_t* buffer, size_t bytesize, size_t byteoffset) {
    pending_wirte.emplace(buffer, bytesize, byteoffset);
}

err_t server_t::ProcessRead() {
    log<log_t::TCP_SERVER_RECEIVED_EVENT>(server_socket.GetFD());
    try {
        auto connection_socket = server_socket.accept();
        auto protocol_implementation = protocol_implementation_creator.create_protocol();
        auto connection = new connection_t(std::move(connection_socket), protocol_implementation);
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

int server_t::GetFD() const { 
    return server_socket.GetFD();
}

} // namespace MMS::net::tcp