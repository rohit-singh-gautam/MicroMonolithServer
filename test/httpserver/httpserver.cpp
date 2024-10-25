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