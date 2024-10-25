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

namespace MMS::http::typecheck {
    template <typename T>
    concept fieldentrypair = std::is_same_v<T, std::pair<MMS::http::FIELD, std::string>>;
} // namespace MMS::typecheck

namespace MMS::server::http {
using MMS::http::VERSION;
using MMS::http::CODE;
using MMS::http::FIELD;
using MMS::http::METHOD;
using MMS::http::request;
using MMS::http::response;

class configuration_t;
class protocol_t;

class handler_t {
public:
    virtual ~handler_t() = default;
    virtual void ProcessRead(const MMS::http::request &request, const std::string &relative_path, protocol_t *writer) = 0;

    // This is expected to return static list hence return type is const reference
    // Following must not be added to list PRI and OPTION
    virtual const std::vector<METHOD> &GetSupportedMethod() = 0;
    virtual bool IsSupported(const http::METHOD method) {
        auto &methods = GetSupportedMethod();
        for(auto methodentry: methods) {
            if (methodentry == method) return true;
        }
        return false;
    }
};

struct configuration_t {
    std::string ServerName;
    prefixmap<std::string, handler_t *> handlermap { };
    uint32_t max_frame_size { 16384 };
    MMS::http::http_version_t version { };
    MMS::http::http_limits_t limits { };
    configuration_t(const std::string &ServerName) : ServerName { ServerName } { }
    configuration_t(configuration_t &&configuration) 
        : ServerName { std::move(configuration.ServerName) }, handlermap { std::move(configuration.handlermap) } { }

    void AddHandler(const std::string &path, handler_t *handler) {
        handlermap.insert(path, handler);
    }
};

class protocol_t : public net::protocol_t {
protected:
    const configuration_t * const configuration;

public:
    protocol_t(const configuration_t *configuration) : configuration { configuration } { assert(configuration); }
    protocol_t(const protocol_t &) = delete;
    protocol_t &operator=(const protocol_t &) = delete;

    using net::protocol_t::Write;


    virtual void WriteError(const CODE code, const std::string &errortext) =  0;
    virtual void Write(const CODE code, const Stream &bodystream, std::vector<std::pair<FIELD, std::string>> &fields) = 0;
    virtual void Write(const CODE code, std::vector<std::pair<FIELD, std::string>> &fields) = 0;

    template <MMS::http::typecheck::fieldentrypair... fieldlist>
    inline void Write(const CODE code, const Stream &bodystream, const fieldlist& ... field) {
        std::vector<std::pair<FIELD, std::string>> fields { field... };
        Write(code, bodystream, fields);
    }

    template <MMS::http::typecheck::fieldentrypair... fieldlist>
    inline void Write(const CODE code, const std::string &body, const fieldlist& ... field) {
        std::vector<std::pair<FIELD, std::string>> fields { field... };
        auto bodystream = make_const_stream(body.c_str(), body.size());
        Write(code, bodystream,  fields);
    }

    template <MMS::http::typecheck::fieldentrypair... fieldlist>
    inline void Write(const CODE code, const fieldlist& ... field) {
        std::vector<std::pair<FIELD, std::string>> fields { field... };
        Write(code, fields);
    }

    static constexpr void CreateSupportedMethodString(std::string &str, const std::vector<METHOD> &supportedmethods, const auto ...AdditionalMethod) {
        for(auto supportedmethod: supportedmethods) {
            str += to_string(supportedmethod);
            str += ", ";
        }
        ((str += to_string(AdditionalMethod) + ", "), ... );
        str.pop_back();
        str.pop_back();
    }

    static constexpr std::string CreateSupportedMethodErrorString(const METHOD requestmethod, const std::vector<METHOD> &supportedmethods, const auto ...AdditionalMethod) {
        std::string errmsg { "Method: " };
        errmsg += to_string(requestmethod);
        errmsg += " not allowed, follow methods are supported:";
        CreateSupportedMethodString(errmsg, supportedmethods, AdditionalMethod...);
        return errmsg;
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

namespace MMS::client::http {
using MMS::http::VERSION;
using MMS::http::CODE;
using MMS::http::FIELD;
using MMS::http::METHOD;
using MMS::http::request;
using MMS::http::response;

class protocol_t;
class handler_t {
public:
    virtual ~handler_t() = default;
    virtual void ProcessRead(const MMS::http::response &response, protocol_t *writer) = 0;
};

class protocol_t : public net::protocol_t {
protected:
    handler_t &handler;
    
public:
    protocol_t(handler_t &handler) : handler { handler } {  }
    protocol_t(const protocol_t &) = delete;
    protocol_t &operator=(const protocol_t &) = delete;

    using net::protocol_t::Write;

    virtual void Write(const METHOD method, const std::string &uri) = 0;
    virtual void Write(const METHOD method, const std::string &uri, const Stream &bodystream, std::vector<std::pair<FIELD, std::string>> &fields) = 0;
    virtual void Write(const METHOD method, const std::string &uri, std::vector<std::pair<FIELD, std::string>> &fields) = 0;

    template <MMS::http::typecheck::fieldentrypair... fieldlist>
    inline void Write(const METHOD method, const std::string &uri, const Stream &bodystream, const fieldlist& ... field) {
        std::vector<std::pair<FIELD, std::string>> fields { field... };
        Write(method, uri, bodystream, fields);
    }

    template <MMS::http::typecheck::fieldentrypair... fieldlist>
    inline void Write(const METHOD method, const std::string &uri, const std::string &body, const fieldlist& ... field) {
        std::vector<std::pair<FIELD, std::string>> fields { field... };
        auto bodystream = make_const_stream(body.c_str(), body.size());
        Write(method, uri, bodystream,  fields);
    }

    template <MMS::http::typecheck::fieldentrypair... fieldlist>
    inline void Write(const METHOD method, const std::string &uri, const fieldlist& ... field) {
        std::vector<std::pair<FIELD, std::string>> fields { field... };
        Write(method, uri, fields);
    }
};

} // namespace MMS::client::http