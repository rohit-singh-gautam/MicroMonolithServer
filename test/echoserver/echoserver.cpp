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
#include <mms/server/echo.h>
#include <mms/net/tcpserver.h>
#include <mms/net/tcpsslserver.h>

int main(int, char *[]) {
    MMS::log<MMS::log_t::APPLICATION_STARTING>();

    const std::filesystem::path filename("/tmp/iotcloud/log/deviceserver.log");

    MMS::server::echocreator_t echoservercreator { };

    const char *cert = "cert/testcert.pem";
    const char *private_key = "cert/testcert.pem";
    MMS::net::ssl::common ssl_common { cert, private_key };

    MMS::listener::listener_t locallistener { filename };
    locallistener.add(new MMS::net::tcp::server_t { 4833, echoservercreator, &locallistener });
    locallistener.add(new MMS::net::tcp::ssl::server_t { 4834, echoservercreator, &locallistener, &ssl_common });
    locallistener.multithread_loop();
    locallistener.wait();

    return 0;
}