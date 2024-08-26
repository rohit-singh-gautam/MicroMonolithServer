/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <json.h>
#include <mms/server/httpfilehandler.h>
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

    bool ReadSystemConfiguration();
    bool ReadHTTPConfiguration();
    bool ReadServerConfiguration();
    bool ReadSSLConfiguration();
    bool ReadProtocolConfiguration();

public:
    Container(listener::listener_t *listener, const std::filesystem::path &path) : listener { listener }, ref { rohit::json::Parse(ReadStringFromFile(path)) } { }
    Container(const Container &) = delete;
    Container &operator=(const Container &) = delete;
    
    bool ReadConfigurations();

    void AddServerToListener();
};


} // namespace MMS::repository