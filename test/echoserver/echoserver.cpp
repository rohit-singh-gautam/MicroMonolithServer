/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <mms/server/echo.h>
#include <thread>

int main(int, char *[]) {
    MMS::log<MMS::log_t::APPLICATION_STARTING>();

    const std::filesystem::path filename("/tmp/iotcloud/log/deviceserver.log");

    MMS::server::echocreator_t echoservercreator { };
    MMS::event::listener_t locallistener { filename };
    MMS::event::tcp::server_t server { 4833, echoservercreator, &locallistener };
    locallistener.add(server);
    locallistener.multithread_loop(4);
    locallistener.wait();

    return 0;
}