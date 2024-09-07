/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/http1.h>
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

    MMS::net::ssl::common ssl_common { cert, private_key };

    std::string_view rootpath { "./www" };
    std::string_view rootpath1 { "./www1" };
    MMS::server::filecache filecache { };

    MMS::server::http::configuration_t configuration { "MicroMonolithServer" };
    std::unique_ptr<MMS::server::http::handler_t> handlerptr { new MMS::server::httpfilehandler { filecache, rootpath, configuration } };
    std::unique_ptr<MMS::server::http::handler_t> handlerptr1 { new MMS::server::httpfilehandler { filecache, rootpath1, configuration } };

    configuration.AddHandler({"/simple" }, std::move(handlerptr));
    configuration.AddHandler({"/" }, std::move(handlerptr1));
    configuration.mimemap.emplace(".html", "text/html");
    configuration.mimemap.emplace(".json", "application/json");
    configuration.mimemap.emplace(".jpeg", "image/jpeg");
    configuration.mimemap.emplace(".jpg", "image/jpeg");
    configuration.mimemap.emplace(".gif", "image/gif");
    configuration.mimemap.emplace(".ico", "image/x-icon");
    configuration.mimemap.emplace(".png", "image/png");

    configuration.defaultlist.emplace_back("index.html");
    configuration.defaultlist.emplace_back("default.html");

    MMS::server::http::v1::creator_t httpcretor { &configuration };
    
    MMS::listener::listener_t locallistener { filename };
    locallistener.add(new MMS::net::tcp::server_t { 80, httpcretor, &locallistener, });
    locallistener.add(new MMS::net::tcp::ssl::server_t { 443, httpcretor, &locallistener, &ssl_common });
    locallistener.multithread_loop();
    locallistener.wait();

    return 0;
}