
/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/httpfilehandler.h>

int main(int, char *[]) {
    MMS::log<MMS::log_t::APPLICATION_STARTING>();

    const std::filesystem::path filename("/tmp/iotcloud/log/deviceserver.log");

    MMS::server::httpcreator_t httpcretor { };
    MMS::server::filecache filecache { };
    std::string_view rootpath { "./www" };
    std::unique_ptr<MMS::server::httphandler_t> handlerptr { new MMS::server::httpfilehandler { filecache, rootpath } };
    httpcretor.AddHandler({"/" }, std::move(handlerptr));

    MMS::event::listener_t locallistener { 4,  filename };
    locallistener.add(new MMS::event::tcp::tcp_server_t { 4833, httpcretor, &locallistener });
    locallistener.multithread_loop();
    locallistener.wait();

    return 0;
}