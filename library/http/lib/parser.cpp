/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http/httpdef.h>

namespace MMS::http {

const std::unordered_map<std::string_view, header::FIELD> header::field_map = {
#define HTTP_FIELD_ENTRY(x, y) {y, header::FIELD::x},
    HTTP_FIELD_LIST
#undef HTTP_FIELD_ENTRY
};

const std::unordered_map<std::string_view, request_header::METHOD> request_header::method_map = {
#define HTTP_METHOD_ENTRY(x) {#x, request_header::METHOD::x},
    HTTP_METHOD_LIST
#undef HTTP_METHOD_ENTRY
};

std::string request::to_string() {
        std::string ret {};
    ret += to_string_view(method);
    ret += " ";
    ret += path;
    ret += " ";
    ret += header::to_string_view(version);
    ret += "\r\n";
    for(auto fieldentry: fields) {
        auto field = fieldentry.first;
        auto value = fieldentry.second;
        ret += header::to_string_view(field);
        ret += ": ";
        ret += value;
        ret += "\r\n";
    }
    if ( !body.empty() ) {
        ret += "\r\n";
        ret += body;
    }
    return ret;
}

std::string_view parse_till_space(const char *&requesttext, size_t &size) {
    auto start = requesttext;
    while(*requesttext != ' ' && size) {
        ++requesttext;
        --size;
    }

    if (start == requesttext) {
        throw MMS::http_parser_failed_t(requesttext, size);
    }

    return { start, static_cast<size_t>(requesttext - start) };
}

std::string_view parse_till_colon(const char *&requesttext, size_t &size) {
    while(*requesttext == ' ' && size) {
        ++requesttext;
        --size;
    }
    auto start = requesttext;
    while(*requesttext != ':' && *requesttext != ' ' && size) {
        ++requesttext;
        --size;
    }
    if (start == requesttext) {
        throw MMS::http_parser_failed_t(requesttext, size);
    }
    std::string_view ret = { start, static_cast<size_t>(requesttext - start) };

    while(*requesttext != ':' && size) {
        ++requesttext;
        --size;
    }

    if (size) {
        // Skip colon
        ++requesttext;
        --size;
    }

    return ret;
}

std::string_view parse_till_CRLF(const char *&requesttext, size_t &size) {
    while(*requesttext == ' ' && size) {
        ++requesttext;
        --size;
    }
    auto start = requesttext;
    auto end = requesttext;
    while(size) {
        if (*requesttext == '\r' || *requesttext == '\n') break;
        if (*(requesttext - 1) != ' ') end = requesttext;
        ++requesttext;
        --size;
    }
    if (*(requesttext - 1) != ' ') end = requesttext;

    std::string_view ret = { start, static_cast<size_t>(end - start) };
    
    if (size) {
        if (*requesttext == '\r') {
            ++requesttext;
            --size;
            if (!size || *requesttext != '\n') throw MMS::http_parser_failed_t(requesttext, size);
            ++requesttext;
            --size;
        } else if(*requesttext == '\n') {
            ++requesttext;
            --size;
        }
    }

    return ret;
}

void parse_skip_one(const char *&requesttext, size_t &size) {
    if (!size) throw MMS::http_parser_failed_t(requesttext, size);
    ++requesttext;
    --size;
}


bool parse_check_CRLF(const char *&requesttext, size_t &size) {
    if (*requesttext == '\r') {
        ++requesttext;
        --size;
        if (!size || *requesttext != '\n') throw MMS::http_parser_failed_t(requesttext, size);
        ++requesttext;
        --size;
        return true;
    } else if(*requesttext == '\n') {
        ++requesttext;
        --size;
        return true;
    }
    return false;
}

class http_parser {
    request &req;
    const char *requesttext;
    size_t size;

public:
    http_parser(request &req, const std::string_view &text) : req { req }, requesttext { text.data() }, size { text.size() } { }

    void parse_method() {
        auto methodtext = parse_till_space(requesttext, size);

        auto itr = req.method_map.find(methodtext);
        if (itr == std::end(req.method_map)) {
            throw MMS::http_parser_failed_t(requesttext, size);
        }

        req.method = itr->second;
    }

    void parse_request_uri() {
        auto requesturi = parse_till_space(requesttext, size);
        req.path = requesturi;
    }

    template <bool crlf_end>
    void parse_version() {
        const auto versiontext = crlf_end ? parse_till_CRLF(requesttext, size) : parse_till_space(requesttext, size);

        if (versiontext.compare("HTTP/1.1") == 0) req.version = header::VERSION::VER_1_1;
        else if (versiontext.compare("HTTP/2.0") == 0) req.version = header::VERSION::VER_1_1;
        else throw MMS::http_parser_failed_t(requesttext, size);
    }

    // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
    // We will allow CR
    void parse_request_line() {
        parse_method();
        parse_skip_one(requesttext, size);
        parse_request_uri();
        parse_skip_one(requesttext, size);
        parse_version<true>();
    }

    void parse_request_fields() {
        while(true) {
            auto field = parse_till_colon(requesttext, size);
            auto value = parse_till_CRLF(requesttext, size);
            auto itr = header::field_map.find(field);
            if (itr != std::end(header::field_map)) {
                // When field is not present in enumeration it will be ignored.
                req.fields[itr->second] = std::move(value);
            }
            if (!size || parse_check_CRLF(requesttext, size)) break;
        }
    }

    size_t parse_request_header() {
        auto start = requesttext;
        parse_request_line();
        if (size) {
            parse_request_fields();
        }
        return static_cast<size_t>(requesttext - start);
    }

    void parse_request() {
        parse_request_line();
        if (size) parse_request_fields();

        // We do not care about content lenght here hence
        // will return entire text in body
        if (size) req.body = std::string_view { requesttext, size };
    }
}; // class http_parser

request::request(const std::string_view &text) {
    http_parser parser {*this, text };
    parser.parse_request();
}

} // namespace MMS::http