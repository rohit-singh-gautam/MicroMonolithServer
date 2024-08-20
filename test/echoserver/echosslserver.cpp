/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <mms/server/echo.h>
#include <mms/net/tcpsslserver.h>
#include <filesystem>

int main(int, char *[]) {
    MMS::log<MMS::log_t::APPLICATION_STARTING>();

    const std::filesystem::path filename("/tmp/iotcloud/log/deviceserver.log");

    MMS::server::echocreator_t echoservercreator { };

    const char *cert = "cert/testcert.pem";
    const char *private_key = "cert/testcert.pem";

    if (!std::filesystem::exists({cert})) {
        std::cout << "Certificate not found \n";
        return 0;
    }

    auto ssl_common = std::make_shared<MMS::net::ssl::common>(cert, private_key);

    MMS::listener::listener_t locallistener { 4,  filename };
    locallistener.add(new MMS::net::tcp::ssl::server_t { 4833, ssl_common, echoservercreator, &locallistener });
    locallistener.multithread_loop();
    locallistener.wait();

    return 0;
}