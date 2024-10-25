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

#include <iostream>
#include <mms/listener.h>
#include <mms/net/tcpserver.h>


int main(int , char *[]) {
    /* if (argc != 4) {
        std::cout << "Usage: echoclient <address> <port> <message>" << std::endl;
        return 0;
    } */
    auto port = MMS::to_ipv6_port_t("8882");
    auto addr = MMS::to_ipv6_addr_t("::1");
    MMS::ipv6_socket_addr_t socketaddr { addr, port };
    MMS::tcp_client_socket_t clientsocket { socketaddr, false };
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