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

    std::vector<std::string> defaultlist { "index.html", "default.html" };
    std::unordered_map<std::string, std::string> minemap {
        {".html", "text/html"},
        {".json", "application/json"},
        {".jpeg", "image/jpeg"},
        {".jpg", "image/jpeg"},
        {".gif", "image/gif"},
        {".ico", "image/x-icon"},
        {".png", "image/png"},
    };

    MMS::server::http::configuration_t configuration { "MicroMonolithServer" };
    MMS::server::httpfilehandler handlerptr { filecache, rootpath, defaultlist, minemap };
    MMS::server::httpfilehandler handlerptr1 { filecache, rootpath1, defaultlist, minemap };

    configuration.AddHandler({"/simple" }, &handlerptr);
    configuration.AddHandler({"/" }, &handlerptr1);

    MMS::server::http::v1::creator_t httpcretor { &configuration };
    
    MMS::listener::listener_t locallistener { filename };
    locallistener.add(new MMS::net::tcp::server_t { 80, httpcretor, &locallistener, });
    locallistener.add(new MMS::net::tcp::ssl::server_t { 443, httpcretor, &locallistener, &ssl_common });
    locallistener.multithread_loop();
    locallistener.wait();

    return 0;
}