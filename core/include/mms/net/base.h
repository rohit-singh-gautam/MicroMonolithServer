/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <cstdint>
#include <netinet/in.h>
#include <mms/listener.h>

namespace MMS::net {
constexpr int socket_backlog { 5 };
int CreateServerSocket(int port);

inline const ipv6_socket_addr_t get_peer_ipv6_addr(const int socket_id) {
    sockaddr_in6 addr;
    socklen_t len = sizeof(addr);
    getpeername(socket_id, reinterpret_cast<struct sockaddr *>(&addr), &len);

    ipv6_port_t &port = *reinterpret_cast<ipv6_port_t *>(&addr.sin6_port);
    return ipv6_socket_addr_t(&addr.sin6_addr.__in6_u, port);
}

inline const ipv6_socket_addr_t get_local_ipv6_addr(const int socket_id) {
    sockaddr_in6 addr;
    socklen_t len = sizeof(addr);
    getsockname(socket_id, (struct sockaddr *)&addr, &len);

    ipv6_port_t &port = *reinterpret_cast<ipv6_port_t *>(&addr.sin6_port);
    return ipv6_socket_addr_t(&addr.sin6_addr.__in6_u, port);
}

class protocol_t {
protected:
    listener::processor_t *processor;

public:
    protocol_t() : processor { nullptr } { }
    protocol_t(listener::processor_t *processor) : processor { processor } { }
    protocol_t(const protocol_t &) = delete;
    protocol_t &operator=(const protocol_t &) = delete;
    virtual ~protocol_t() = default;

    void SetProcessor(listener::processor_t *processor) { this->processor = processor; }

    virtual void ProcessRead(const ConstStream &stream) = 0;

    template <bool CopyBuffer = true, typename buffertype>
    inline void Write(buffertype buffer, size_t bytesize, size_t byteoffset = 0) {
        processor->Write<CopyBuffer, buffertype>(buffer, bytesize, byteoffset);
    }

    inline void Write(const std::string &buffer) {
        processor->Write(buffer);
    }

    inline void Write(const std::string_view &buffer) {
        processor->Write(buffer);
    }


    template <listener::typecheck::write_entry_const... buffertype>
    inline void Write(const buffertype&... buffer) {
        processor->Write<buffertype...>(buffer...);
    }

    constexpr inline auto GetFD() const { return processor->GetFD(); }
}; // protocol_t

class protocol_creator_t {
public:
    virtual ~protocol_creator_t() = default;
    virtual protocol_t *create_protocol() = 0;
}; // protocol_creator_t


} // namespace MMS::net