/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string_view>
#include <unordered_map>
#include <mms/base/error.h>
#include <http/httpdef.h>

#ifndef LIST_DEFINITION_END
#define LIST_DEFINITION_END
#endif

namespace MMS::http {
/*
    GET /hello.txt HTTP/1.1
    Host: www.example.com
    User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3
    Accept-Language: en, mi


    https://www.rfc-editor.org/rfc/rfc2616
    HTTP/1.1 206 Partial content
    Date: Wed, 15 Nov 1995 06:25:24 GMT
    Last-Modified: Wed, 15 Nov 1995 04:58:08 GMT
    Content-Range: bytes 21010-47021/47022
    Content-Length: 26012
    Content-Type: image/gif


*/

class header {
protected:

    const std::string_view empty { };
    VERSION version { };
    fields_t fields { };

    template <bool crlf_end>
    void parse_version(const char *&, size_t &);
    void parse_fields(const char *&, size_t &);

public:
    constexpr header() { }
    constexpr header(VERSION version) : version { version } { }

        const auto  &GetField(FIELD field) const {
        const auto field_itr = fields.find(field);
        if (field_itr != fields.end()) {
            return field_itr->second;
        }
        return empty;
    }
}; // class header

class request_header : public header {
protected:
    constexpr request_header() { }
    constexpr request_header(VERSION version) : header(version) {}

    METHOD method { };
    std::string_view path { };

    void parse_request_uri(const char *&, size_t &);
    void parse_method(const char *&, size_t &);
    void parse_request_line(const char *&, size_t &);
public: 
    constexpr auto GetMethod() const { return method; }
    constexpr const auto &GetPath() const { return path; }

    auto upgrade_version() {
        auto field_itr = fields.find(FIELD::Upgrade);
        if (field_itr != fields.end()) {
            if (field_itr->second == "h2c") {
                return VERSION::VER_2;
            }
        }

        return VERSION::VER_1_1;
    }

}; // class request_header

class request : public request_header {
    friend class http_request_parser;
    std::string_view body { };

public:
    request(const std::string_view &);

    const auto &GetBody() const { return body; }

    void parse(const char *&, size_t &);

    std::string to_string();
}; // class request

class response_header : public header {
protected:
    CODE code { };
    response_header() { }
    response_header(VERSION version) : header { version } { }

    void parse_code(const char *&, size_t &);
    void parse_response_line(const char *&, size_t &);
public:
    constexpr auto GetCode() const { return code; };
};

class response : public response_header {
    std::string_view body { };

public:
    response(const std::string_view &);

    const auto &GetBody() const { return body; }

    void parse(const char *&, size_t &);

    std::string to_string();
}; // response

} // namespace MMS::http