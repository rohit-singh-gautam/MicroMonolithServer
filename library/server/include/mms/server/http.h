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
using MMS::http::CODE;
using MMS::http::FIELD;
using MMS::http::METHOD;
using MMS::http::request;
using MMS::http::response;

namespace typecheck {
template <typename T>
concept fieldentrypair = std::is_same_v<T, std::pair<FIELD, std::string>>;
} // namespace typecheck

class configuration_t;
class protocol_t;

class handler_t {
public:
    virtual ~handler_t() = default;
    virtual void ProcessRead(const MMS::http::request &request, const std::string &relative_path, protocol_t *writer) = 0;
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
    const configuration_t * const configuration;

    friend class creator_t;
    protocol_t(const configuration_t *configuration) : configuration { configuration } { assert(configuration); }

    request *current_request { nullptr };

public:
    protocol_t(const protocol_t &) = delete;
    protocol_t &operator=(const protocol_t &) = delete;

    void ProcessRead(const uint8_t *buffer, const size_t size) override;

    using net::protocol_t::Write;

    inline  void Write(const MMS::http::response &response) {
        Write(response.to_string());
    }

    inline void Write(const MMS::http::response &response, const std::string &body) {
        Write(
            listener::write_entry_const { response.to_string()},
            listener::write_entry_const { body });
    }

    inline void Write(const MMS::http::response &response, const char *bodybuffer, size_t bodysize) {
        Write(
            listener::write_entry_const { response.to_string()},
            listener::write_entry_const { bodybuffer, bodysize });
    }

    inline void WriteError(const CODE code, const std::string &errortext) {
        if (current_request) {
            auto res = current_request->CreateErrorResponse(code, errortext, configuration->ServerName);
            Write(res);
        } else {
            auto res = response::CreateErrorResponse(code, errortext, configuration->ServerName);
            Write(res);
        }
    }

    template <typecheck::fieldentrypair... fieldlist>
    inline void Write(const CODE code, const char *bodybuffer, size_t bodysize, const fieldlist& ... field) {
        assert(current_request);
        auto response = response::CreateBasicResponse(code);
        (response.add_field(field), ...);
        response.add_field(MMS::http::FIELD::Server, configuration->ServerName);
        response.add_field(FIELD::Content_Length, bodysize);
        Write(response, bodybuffer, bodysize);
    }


    template <typecheck::fieldentrypair... fieldlist>
    inline void Write(const CODE code, const std::string &body, const fieldlist& ... field) {
        assert(current_request);
        auto response = response::CreateBasicResponse(code);
        (response.add_field(field), ...);
        response.add_field(FIELD::Content_Length, std::to_string(body.size()));
        Write(response, body);
    }
};

class creator_t : public net::protocol_creator_t {
    const configuration_t *configuration;

public:
    creator_t(configuration_t *configuration) : configuration { configuration } { }
    creator_t(const creator_t &) = delete;
    creator_t &operator=(const creator_t &) = delete;

    net::protocol_t *create_protocol() override { return new protocol_t { configuration }; }
};

} // namespace MMS::server::http
