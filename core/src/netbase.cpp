
/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/net/base.h>
#include <mms/base/types.h>
#include <mms/log/log.h>
#include <mms/base/error.h>
#include <netinet/in.h>

namespace MMS {

namespace net {
int CreateServerSocket(int port) {
    const int socket_id = socket(AF_INET6, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_TCP);
    int enable = 1;
    if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char *)&enable,sizeof(enable)) < 0) {
        ::close(socket_id); 
        throw setsockopt_fail_t(error_helper_t::sockopt_ret());
    }

    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    addr.sin6_addr = in6addr_any;

    if (bind(socket_id, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(socket_id);
        throw bind_fail_t { };
    }
    log<log_t::TCP_SOCKET_BIND_SUCCESS>(socket_id, port);

    if (listen(socket_id, socket_backlog) < 0) {
        ::close(socket_id);
        throw listen_fail_t { };
    }
    log<log_t::TCP_SOCKET_LISTEN_SUCCESS>(socket_id, port);

    return socket_id;
}
} // namespace net

ipv6_socket_addr_t::operator sockaddr_in6() const {
    sockaddr_in6 sockaddr = {};
    sockaddr.sin6_family = AF_INET6;
    sockaddr.sin6_addr.__in6_u.__u6_addr32[0] = addr.addr_32[0];
    sockaddr.sin6_addr.__in6_u.__u6_addr32[1] = addr.addr_32[1];
    sockaddr.sin6_addr.__in6_u.__u6_addr32[2] = addr.addr_32[2];
    sockaddr.sin6_addr.__in6_u.__u6_addr32[3] = addr.addr_32[3];
    sockaddr.sin6_port = port.get_network_port();
    return sockaddr;
}
} // namespace MMS