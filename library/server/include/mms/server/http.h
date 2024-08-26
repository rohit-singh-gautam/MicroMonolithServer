/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/net/base.h>
#include <mms/ds/prefixmap.h>
#include <http/httpparser.h>
#include <unordered_map>

namespace MMS::server::http {

class writer_t {
    listener::writer_t &connectionwriter;
public:
    writer_t(listener::writer_t &connectionwriter) : connectionwriter { connectionwriter } { }
    void Write(const MMS::http::response &response);
};

class configuration_t;

class handler_t {
public:
    virtual ~handler_t() = default;
    virtual void ProcessRead(const MMS::http::request &request, const std::string &relative_path, listener::writer_t &writer) = 0;
};

struct configuration_t {
    std::string ServerName;
    prefixmap<std::string, std::unique_ptr<handler_t>> handlermap { };
    std::unordered_map<std::string, std::string> mimemap { };
    std::vector<std::string> defaultlist { };
    configuration_t(const std::string &ServerName) : ServerName { ServerName } { }
    configuration_t(configuration_t &&configuration) 
        : ServerName { std::move(configuration.ServerName) }, handlermap { std::move(configuration.handlermap) }, 
            mimemap { std::move(configuration.mimemap) }, defaultlist { std::move(defaultlist) } { }

    void AddHandler(const std::string &path, std::unique_ptr<handler_t> &&handler) {
        handlermap.insert(path, std::move(handler));
    }
};

class creator_t;
class protocol_t : public net::protocol_t {
    const configuration_t &configuration;

    friend class creator_t;
    protocol_t(const configuration_t &configuration) : configuration { configuration } { }

public:
    void ProcessRead(const uint8_t *buffer, const size_t size, listener::writer_t &writer) override;
};

class creator_t : public net::protocol_creator_t {
    const configuration_t &configuration;

public:
    creator_t(configuration_t &configuration) : configuration { configuration } { }
    net::protocol_t *create_protocol() override { return new protocol_t { configuration }; }
};

} // namespace MMS::server::http