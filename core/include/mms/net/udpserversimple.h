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
#include <mms/net/tcpcommon.h>
#include <sys/socket.h>

namespace MMS::net::udp {


class server_t : public listener::processor_t {
    std::unique_ptr<protocol_t> protocol_implementation;
    std::queue<std::pair<sockaddr_in6, FixedBuffer>> pending_wirte { };

    sockaddr_in6 *current_client_addr { nullptr };

public:
    server_t(const int port, protocol_creator_t &protocol_creator, listener::listener_t *)
        : listener::processor_t { CreateUDPServerSocket(port) }, 
            protocol_implementation { protocol_creator.create_protocol(GetFD(), { }) }
    {
        protocol_implementation->SetProcessor(this);
    }
    virtual ~server_t() = default;
    server_t(const server_t &) = default;
    server_t &operator=(const server_t &) = default;
    err_t ProcessRead() override;
    err_t ProcessWrite() override;
    void WriteNoCopy(FixedBuffer &&) override;
}; // server_t

} // namespace MMS::net::tcp