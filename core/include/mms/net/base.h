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

struct buffer_t {
public:
    static constexpr size_t initial_size { 256 };

private:
    void *buffer;
    size_t size;

public:
    buffer_t(const size_t size = initial_size) : buffer { malloc(size)}, size { size } { }
    
    buffer_t(const buffer_t &) = delete;
    buffer_t &operator=(const buffer_t &) = delete;

    ~buffer_t() {
        if (buffer) free(buffer);
    }

    void increase_buffer(const size_t newsize) {
        if (newsize > size) {
            buffer = realloc(buffer, newsize);
            size = newsize;
        }
    }

    template <typename type>
    auto GetBuffer() { return reinterpret_cast<type>(buffer); }

    auto GetBuffer(const size_t offset) {
        if (offset + initial_size > size) {
            increase_buffer(size + initial_size);
        }
        auto newsize = size - offset;
        auto bufferoffset = reinterpret_cast<uint8_t *>(buffer) + offset;
        return std::make_pair(reinterpret_cast<void *>(bufferoffset), newsize);
    }


}; // buffer_t

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

    virtual void ProcessRead(const uint8_t *buffer, const size_t size) = 0;

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