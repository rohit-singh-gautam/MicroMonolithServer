/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <mms/event/listener.h>
#include <mms/net/tcpserverevent.h>


int main(int , char *[]) {
    /* if (argc != 4) {
        std::cout << "Usage: echoclient <address> <port> <message>" << std::endl;
        return 0;
    } */
    auto port = MMS::to_ipv6_port_t("4833");
    auto addr = MMS::to_ipv6_addr_t("::1");
    MMS::ipv6_socket_addr_t socketaddr { addr, port };
    MMS::client_socket_t clientsocket { socketaddr };
    const char *message { "Test" };
    auto message_len = strlen(message);
    size_t actualsent { 0 };
    MMS::err_t ret = clientsocket.write_wait(message, message_len, actualsent);
    std::cout << "Written: " << message << " readlen: " << message_len << " Error: " << ret << std::endl;
    const size_t buffersize { 256 };
    char buffer[buffersize] { };
    size_t offset { 0 };
    size_t readlen { 0 };
    do {
        ret = clientsocket.read(buffer + offset, buffersize  - offset, readlen);
        offset += readlen;
    } while(ret != MMS::err_t::SUCCESS);

    std::cout << "Received: \"" << buffer << "\" readlen: " << readlen << " Error: " << ret << std::endl;
    std::cout << "Received: \"" << buffer << "\" readlen: " << readlen << std::endl;
    
    return 0;
}