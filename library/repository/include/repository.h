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
#pragma once
#include <json.h>
#include <mms/server/httpfilehandler.h>
#include <mms/server/ServiceBase.h>
#include <mms/net/tcpserver.h>
#include <mms/net/tcpsslserver.h>
#include <filesystem>
#include <unordered_map>
#include <memory>

namespace MMS::repository {

std::string ReadStringFromFile(const std::filesystem::path &path);

class Container {
    listener::listener_t *listener;
    rohit::json::Ref ref;
    MMS::server::filecache filecache { };
    std::unordered_map<std::string, std::unique_ptr<server::http::configuration_t>> httpconfigurations { };
    std::unordered_map<std::string, std::unique_ptr<net::ssl::common>> SSLConfigurations { };
    std::unordered_map<std::string, std::unique_ptr<listener::processor_t>> Servers { };
    std::unordered_map<std::string, std::unique_ptr<net::protocol_creator_t>> protocols { };
    std::unordered_map<std::string, std::unique_ptr<server::http::handler_t>> handlers { };
    std::unordered_map<std::string, std::unique_ptr<server::ServiceBase>> services { };
    std::unordered_map<std::string, std::vector<std::string>> string_lists { };
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> string_maps { };


    bool ReadSystemConfiguration();
    bool ReadHTTPConfiguration();
    bool ReadServerConfiguration();
    bool ReadSSLConfiguration();
    bool ReadProtocolConfiguration();
    bool ReadHandlerConfiguration();
    bool ReadData();

public:
    Container(listener::listener_t *listener, const std::filesystem::path &path) : listener { listener }, ref { rohit::json::Parse(ReadStringFromFile(path)) } { }
    Container(const Container &) = delete;
    Container &operator=(const Container &) = delete;
    
    bool ReadConfigurations();

    void AddServerToListener();
};


} // namespace MMS::repository