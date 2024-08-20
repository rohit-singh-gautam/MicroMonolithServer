/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/listener.h>
#include <mms/net/sslcommon.h>
#include <mms/net/base.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <memory>


namespace MMS::net::tcp::ssl {

class connection_t : public listener::processor_t {
    SSL *ssl;

    protocol_t *const protocol_implementation;
    std::queue<listener::write_entry> pending_wirte { };
    
    static thread_local buffer_t tempbuffer;

public:
    connection_t(int fd, SSL *ssl, protocol_t * const protocol_implementation)
        : processor_t { fd }, ssl { ssl }, protocol_implementation { protocol_implementation } { }
    ~connection_t();
    
    connection_t(const connection_t&) = delete;
    connection_t& operator=(const connection_t&) = delete;

    err_t ProcessRead() override;
    err_t ProcessWrite() override;
    void WriteNoCopy(uint8_t* buffer, size_t bytesize, size_t byteoffset) override;

    auto get_peer_ipv6_addr() const { return MMS::net::get_peer_ipv6_addr(GetFD()); }
}; // connection_t

class server_t : public listener::processor_t {
    static constexpr int socket_backlog { 5 };
    protocol_creator_t &protocol_creator;
    listener::listener_t *listener;

    static int CreateServerSocket(int port);

    std::shared_ptr<MMS::net::ssl::common> ssl_common;


public:
    server_t(const int port, std::shared_ptr<MMS::net::ssl::common> &ssl_common, protocol_creator_t &protocol_creator, listener::listener_t *listener)
        : listener::processor_t { CreateServerSocket(port) }, protocol_creator { protocol_creator }, listener { listener }, ssl_common { ssl_common } 
    { }

    server_t(const server_t &) = default;
    server_t &operator=(const server_t &) = default;
    err_t ProcessRead() override;
}; // server_t


}; // namespace MMS::net::tcp::ssl