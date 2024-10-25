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
#include <mms/net/sslcommon.h>
#include <mms/net/base.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <memory>
#include <mms/net/tcpcommon.h>


namespace MMS::net::tcp::ssl {

class connection_t : public connection_base_t {
protected:
    SSL *ssl;
    size_t writeoffset { 0 };
    
public:
    connection_t(int fd, SSL *ssl, protocol_t *protocol_implementation)
        : connection_base_t { fd, protocol_implementation }, ssl { ssl } { }
    ~connection_t();
    
    connection_t(const connection_t&) = delete;
    connection_t& operator=(const connection_t&) = delete;

    err_t ProcessRead() override;
    err_t ProcessWrite() override;
}; // connection_t

class server_t : public listener::processor_t {
    static constexpr int socket_backlog { 5 };
    protocol_creator_t &protocol_creator;
    listener::listener_t *listener;
    MMS::net::ssl::common *const ssl_common;


public:
    server_t(const int port, protocol_creator_t &protocol_creator, listener::listener_t *listener, MMS::net::ssl::common *ssl_common)
        : listener::processor_t { CreateTCPServerSocket(port) }, protocol_creator { protocol_creator }, listener { listener }, ssl_common { ssl_common }
    { }

    server_t(const server_t &) = default;
    server_t &operator=(const server_t &) = default;
    err_t ProcessRead() override;

    static const std::string_view get_protocol(SSL *ssl);
}; // server_t


}; // namespace MMS::net::tcp::ssl