/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/httpfilehandler.h>
#include <mms/net/tcpserver.h>
#include <mms/net/tcpsslserver.h>

int main(int, char *[]) {
    MMS::log<MMS::log_t::APPLICATION_STARTING>();

    const std::filesystem::path filename("/tmp/iotcloud/log/deviceserver.log");

    const char *cert = "cert/testcert.pem";
    const char *private_key = "cert/testcert.pem";

    if (!std::filesystem::exists({cert})) {
        std::cout << "Certificate not found \n";
        return 0;
    }

    auto ssl_common = std::make_shared<MMS::net::ssl::common>(cert, private_key);

    std::string_view rootpath { "./www" };
    MMS::server::filecache filecache { };

    MMS::server::http::configuration_t configuration { "MicroMonolithServer" };
    std::unique_ptr<MMS::server::http::handler_t> handlerptr { new MMS::server::httpfilehandler { filecache, rootpath, configuration } };

    configuration.AddHandler({"/" }, std::move(handlerptr));
    configuration.mimemap.emplace(".html", "text/html");
    configuration.mimemap.emplace(".json", "application/json");
    configuration.defaultlist.emplace_back("index.html");
    configuration.defaultlist.emplace_back("default.html");

    MMS::server::http::creator_t httpcretor { configuration };
    
    MMS::listener::listener_t locallistener { 4,  filename };
    locallistener.add(new MMS::net::tcp::server_t { 80, httpcretor, &locallistener });
    locallistener.add(new MMS::net::tcp::ssl::server_t { 443, ssl_common, httpcretor, &locallistener });
    locallistener.multithread_loop();
    locallistener.wait();

    return 0;
}