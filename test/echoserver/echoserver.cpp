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
    MMS::event::listener_t locallistener { 4,  filename };
    locallistener.add(new MMS::event::tcp::tcp_server_t { 4833, echoservercreator, &locallistener });
    locallistener.multithread_loop();
    locallistener.wait();

    return 0;
}