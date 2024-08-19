/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/listener.h>


namespace MMS::net::tcp::ssl {

class connection_t : public listener::processor_t {
    tcp_socket_t connection_socket;
    protocol_t *const protocol_implementation;
    std::queue<listener::write_entry> pending_wirte { };

    static thread_local buffer_t tempbuffer;

public:
    connection_t(tcp_socket_t &&connection_socket, protocol_t * const protocol_implementation)
        : connection_socket { std::move(connection_socket) }, protocol_implementation { protocol_implementation } { }
    
    connection_t(const connection_t&) = delete;
    connection_t& operator=(const connection_t&) = delete;

    err_t ProcessRead() override;
    err_t ProcessWrite() override;
    int GetFD() const override;
    void WriteNoCopy(uint8_t* buffer, size_t bytesize, size_t byteoffset) override;

    auto get_peer_ipv6_addr() const { return connection_socket.get_peer_ipv6_addr(); }
}; // connection_t


}; // namespace MMS::net::tcp::ssl