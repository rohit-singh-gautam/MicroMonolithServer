/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/base/stream.h>
#include <string_view>
#include <unordered_map>
#include <mms/base/error.h>
#include <mms/log/log.h>
#include <http/httpdef.h>

#ifndef LIST_DEFINITION_END
#define LIST_DEFINITION_END
#endif

namespace MMS::http {
class header {
protected:
    static const std::string empty;
    VERSION version { };
    fields_t fields { };

    template <bool crlf_end>
    void parse_version(const ConstFullStream &);
    void parse_fields(const ConstFullStream &);

public:
    constexpr header() { }
    constexpr header(VERSION version) : version { version } { }
    header(header &&other) : version { other.version }, fields { std::move(other.fields) } { }

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

    void add_field(const std::pair<FIELD, std::string> &entry) {
        fields.insert(entry);
    }

    void add_field(const FIELD field, std::string &&value) {
        fields.emplace(field, std::move(value));
    }

    void add_field(const FIELD field, const size_t value) {
        fields.emplace(field, std::to_string(value));
    }

protected:
    void parse_request_uri(const ConstFullStream &);
    void parse_method(const ConstFullStream &);
    void parse_request_line(const ConstFullStream &);

    void SetMethod(const METHOD method) { fields.emplace(FIELD::Method, to_string(method)); }
    void SetMethod(const std::string &method) { fields.emplace(FIELD::Method, method); }
    void SetMethod(const std::string &&method) { fields.emplace(FIELD::Method, std::move(method)); }
    void SetPath(const std::string &path) { fields.emplace(FIELD::Path, path); }
    void SetPath(const std::string &&path) { fields.emplace(FIELD::Path, std::move(path)); }

public: 
    constexpr auto GetMethod() const {
        auto field_itr = fields.find(FIELD::Method);
        if (field_itr != fields.end()) {
            return to_method(field_itr->second);
        }
        return METHOD::IGNORE_THIS;
    }

    constexpr auto GetMethodStr() const {
        auto field_itr = fields.find(FIELD::Method);
        if (field_itr != fields.end()) {
            return field_itr->second;
        }
        return empty;
    }

    constexpr const auto &GetPath() const { 
        auto field_itr = fields.find(FIELD::Path);
        if (field_itr != fields.end()) {
            return field_itr->second;
        }
        return empty;
     }

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
class request : public header {
protected:
    using header::header;

    friend class http_request_parser;
    std::string body { };

public:
    request(const ConstFullStream &);
    request(request &&other) : header { std::move(other) }, body { std::move(body) } { }

    constexpr const auto &GetBody() const { return body; }

    void parse(const ConstFullStream &);

    response CreateErrorResponse(CODE code, const std::string &errortext, const std::string &servername) const;

    std::string to_string();
}; // class request

class response_header : public header {
protected:
    CODE code { };
    constexpr response_header() { }
    constexpr response_header(VERSION version) : header { version } { }

    void parse_code(const ConstFullStream &);
    void parse_response_line(const ConstFullStream &);
public:
    constexpr auto GetCode() const { return code; };
};

class response : public response_header {
    friend class request;
    std::string body { };

public:
    constexpr response() { }
    response(const ConstFullStream &);

    constexpr void UpdateContentLength() {
        if (!body.empty()) {
            fields[FIELD::Content_Length] = std::to_string(body.size());
        }
    }

    static response CreateBasicResponse(CODE code);
    static response CreateErrorResponse(CODE code, const std::string &errortext, const std::string &servername);

    constexpr const auto &GetBody() const { return body; }

    auto SetBody(const std::string &value) { body = value; }
    auto SetBody(std::string &&value) { body = std::move(value); }

    void parse(const ConstFullStream &);

    std::string to_string() const;
}; // response

} // namespace MMS::http
