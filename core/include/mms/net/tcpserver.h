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
#include <mms/net/tcpcommon.h>

namespace MMS::net::tcp {

class connection_t : public connection_base_t {
public:
    using connection_base_t::connection_base_t;
    connection_t(const connection_t&) = delete;
    connection_t& operator=(const connection_t&) = delete;

    err_t ProcessRead() override;
    err_t ProcessWrite() override;
}; // connection_t

class server_t : public listener::processor_t {
    protocol_creator_t &protocol_creator;
    listener::listener_t *listener;

public:
    server_t(const int port, protocol_creator_t &protocol_creator, listener::listener_t *listener)
        : listener::processor_t { CreateServerSocket(port) }, 
            protocol_creator { protocol_creator }, listener { listener } { }
    server_t(const server_t &) = default;
    server_t &operator=(const server_t &) = default;
    err_t ProcessRead() override;
}; // server_t

} // namespace MMS::net::tcp