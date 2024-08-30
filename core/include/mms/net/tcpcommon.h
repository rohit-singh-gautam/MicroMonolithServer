/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/listener.h>
#include <mms/base/types.h>
#include <mms/net/socket.h>
#include <mms/net/base.h>
#include <memory>

namespace MMS::net::tcp {

class connection_base_t : public listener::processor_t {
protected:
    std::unique_ptr<protocol_t> protocol_implementation;
    std::queue<listener::write_entry> pending_wirte { };

    static constexpr size_t initial_buffer_size { 256 };
    static thread_local FullStreamAutoAlloc tempbuffer;

public:
    connection_base_t(int fd, protocol_t *protocol_implementation)
        : processor_t { fd }, protocol_implementation { protocol_implementation } { }
    
    connection_base_t(const connection_base_t&) = delete;
    connection_base_t& operator=(const connection_base_t&) = delete;

    void SetProtocol(protocol_t *protocol) {
        protocol_implementation.reset(protocol);
    }

    void WriteNoCopy(uint8_t* buffer, size_t bytesize, size_t byteoffset) override;

    auto get_peer_ipv6_addr() const { return MMS::net::get_peer_ipv6_addr(GetFD()); }
}; // connection_base_t

} // namespace MMS::net::tcp