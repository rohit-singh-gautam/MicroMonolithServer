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

#pragma once
#include <cstdint>
#include <netinet/in.h>
#include <mms/listener.h>

namespace MMS::net {
constexpr int socket_backlog { 5 };
int CreateTCPServerSocket(int port);
int CreateUDPServerSocket(int port);

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

    virtual void ProcessRead(const Stream &stream) = 0;

    void WriteNoCopy(FixedBuffer &&buffer) { processor->WriteNoCopy(std::move(buffer)); };

    inline void Write(const std::string &buffer) {
        processor->Write(buffer);
    }

    inline void Write(const std::string_view &buffer) {
        processor->Write(buffer);
    }

    template <typecheck::WriteStream... buffertype>
    inline void Write(const buffertype&... buffer) {
        processor->Write<buffertype...>(buffer...);
    }

    constexpr inline auto GetFD() const { return processor->GetFD(); }
}; // protocol_t

class protocol_creator_t {
public:
    virtual ~protocol_creator_t() = default;
    virtual protocol_t *create_protocol(int logid, const std::string_view &protoname = { }) = 0;
}; // protocol_creator_t


} // namespace MMS::net