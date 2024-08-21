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
class header {
protected:

    const std::string empty { };
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

    void add_field(const FIELD field, const std::string &value) {
        fields.emplace(field, value);
    }

    void add_field(const FIELD field, std::string &&value) {
        fields.emplace(field, std::move(value));
    }


}; // class header

class request_header : public header {
protected:
    constexpr request_header() { }
    constexpr request_header(VERSION version) : header(version) {}

    METHOD method { };
    std::string path { };

    void parse_request_uri(const char *&, size_t &);
    void parse_method(const char *&, size_t &);
    void parse_request_line(const char *&, size_t &);
public: 
    constexpr auto GetMethod() const { return method; }
    constexpr const auto &GetPath() const { return path; }

    constexpr auto upgrade_version() {
        auto field_itr = fields.find(FIELD::Upgrade);
        if (field_itr != fields.end()) {
            if (field_itr->second == "h2c") {
                return VERSION::VER_2;
            }
        }

        return VERSION::VER_1_1;
    }

}; // class request_header

class response;
class request : public request_header {
    friend class http_request_parser;
    std::string body { };

public:
    request(const std::string &);

    constexpr const auto &GetBody() const { return body; }

    void parse(const char *&, size_t &);

    response CreateErrorResponse(CODE code, const std::string &errortext) const;

    std::string to_string();
}; // class request

class response_header : public header {
protected:
    CODE code { };
    constexpr response_header() { }
    constexpr response_header(VERSION version) : header { version } { }

    void parse_code(const char *&, size_t &);
    void parse_response_line(const char *&, size_t &);
public:
    constexpr auto GetCode() const { return code; };
};

class response : public response_header {
    friend class request;
    std::string body { };

public:
    constexpr response() { }
    response(const std::string &);

    constexpr void UpdateContentLength() {
        if (!body.empty()) {
            fields[FIELD::Content_Length] = std::to_string(body.size());
        }
    }

    static response CreateBasicResponse(CODE code);
    static response CreateErrorResponse(CODE code, const std::string &errortext);

    constexpr const auto &GetBody() const { return body; }

    auto SetBody(const std::string &value) { body = value; }
    auto SetBody(std::string &&value) { body = std::move(value); }

    void parse(const char *&, size_t &);

    std::string to_string() const;
}; // response

} // namespace MMS::http