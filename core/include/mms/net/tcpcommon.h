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
#include <mms/listener.h>
#include <mms/base/types.h>
#include <mms/net/socket.h>
#include <mms/net/base.h>
#include <memory>

namespace MMS::net::tcp {

class connection_base_t : public listener::processor_t {
protected:
    std::unique_ptr<protocol_t> protocol_implementation;
    std::queue<FixedBuffer> pending_wirte { };

public:
    connection_base_t(int fd, protocol_t *protocol_implementation)
        : processor_t { fd }, protocol_implementation { protocol_implementation } { }
    
    connection_base_t(const connection_base_t&) = delete;
    connection_base_t& operator=(const connection_base_t&) = delete;

    void SetProtocol(protocol_t *protocol) {
        protocol_implementation.reset(protocol);
    }

    void WriteNoCopy(FixedBuffer &&buffer) override;

    auto get_peer_ipv6_addr() const { return MMS::net::get_peer_ipv6_addr(GetFD()); }
}; // connection_base_t

} // namespace MMS::net::tcp