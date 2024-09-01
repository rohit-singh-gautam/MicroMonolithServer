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
using MMS::http::VERSION;
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
    uint32_t max_frame_size { 16384 };
    bool http1 { true };
    bool http2 { true };
    bool http2pri { true };
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
protected:
    const configuration_t * const configuration;

public:
    protocol_t(const configuration_t *configuration) : configuration { configuration } { assert(configuration); }
    protocol_t(const protocol_t &) = delete;
    protocol_t &operator=(const protocol_t &) = delete;

    using net::protocol_t::Write;


    virtual void WriteError(const CODE code, const std::string &errortext) =  0;
    virtual void Write(const CODE code, const ConstStream &bodystream, std::vector<std::pair<FIELD, std::string>> &fields) = 0;
    virtual void Write(const CODE code, std::vector<std::pair<FIELD, std::string>> &fields) = 0;

    template <typecheck::fieldentrypair... fieldlist>
    inline void Write(const CODE code, const ConstStream &bodystream, const fieldlist& ... field) {
        std::vector<std::pair<FIELD, std::string>> fields { field... };
        Write(code, bodystream, fields);
    }

    template <typecheck::fieldentrypair... fieldlist>
    inline void Write(const CODE code, const std::string &body, const fieldlist& ... field) {
        std::vector<std::pair<FIELD, std::string>> fields { field... };
        ConstStream bodystream { body.c_str(), body.size() };
        Write(code, bodystream,  fields);
    }

    template <typecheck::fieldentrypair... fieldlist>
    inline void Write(const CODE code, const fieldlist& ... field) {
        std::vector<std::pair<FIELD, std::string>> fields { field... };
        Write(code, fields);
    }
};

class creator_t : public net::protocol_creator_t {
protected:
    const configuration_t *configuration;

public:
    creator_t(configuration_t *configuration) : configuration { configuration } { }
    creator_t(const creator_t &) = delete;
    creator_t &operator=(const creator_t &) = delete;
};

} // namespace MMS::server::http
