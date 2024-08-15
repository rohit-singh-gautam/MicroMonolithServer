/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http/httpparser.h>
#include <format>

namespace MMS::http {

const std::unordered_map<std::string_view, FIELD> field_map = {
#define HTTP_FIELD_ENTRY(x, y) {y, FIELD::x},
    HTTP_FIELD_LIST
#undef HTTP_FIELD_ENTRY
};

const std::unordered_map<std::string_view, METHOD> method_map = {
#define HTTP_METHOD_ENTRY(x) {#x, METHOD::x},
    HTTP_METHOD_LIST
#undef HTTP_METHOD_ENTRY
};

const std::unordered_map<std::string_view, CODE> code_map_raw = {
#define HTTP_CODE_ENTRY(x, y) {#x, CODE::_##x},
    HTTP_CODE_LIST
#undef HTTP_CODE_ENTRY
};

std::string request::to_string() {
    std::string ret {};
    ret += to_string_view(method);
    ret += " ";
    ret += path;
    ret += " ";
    ret += to_string_view(version);
    ret += "\r\n";
    for(auto fieldentry: fields) {
        auto field = fieldentry.first;
        auto value = fieldentry.second;
        ret += to_string_view(field);
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

std::string response::to_string() {
    std::string ret = std::format(
        "{} {} {}\r\n",
        to_string_view(version),
        static_cast<int>(code),
        to_string_view(code)
    );

    for(auto fieldentry: fields) {
        auto field = fieldentry.first;
        auto value = fieldentry.second;
        ret += to_string_view(field);
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

template <bool crlf_end>
void header::parse_version(const char *&requesttext, size_t &size) {
    const auto versiontext = crlf_end ? parse_till_CRLF(requesttext, size) : parse_till_space(requesttext, size);
    version = to_version(versiontext);
    if (version == VERSION::VER_UNKNOWN) throw MMS::http_parser_failed_t(requesttext, size);
}

void header::parse_fields(const char *&requesttext, size_t &size) {
    while(true) {
        auto fieldtext = parse_till_colon(requesttext, size);
        auto value = parse_till_CRLF(requesttext, size);
        auto field = to_field(fieldtext);
        // When field is not present in enumeration it will be ignored.
        if (field != FIELD::FIELD_UNKNOWN) fields[field] = std::move(value);
        if (!size || parse_check_CRLF(requesttext, size)) break;
    }
}

void request_header::parse_method(const char *&requesttext, size_t &size) {
    auto methodtext = parse_till_space(requesttext, size);
    method = to_method(methodtext);
    if (method == METHOD::IGNORE_THIS) throw MMS::http_parser_failed_t(requesttext, size);
}

void request_header::parse_request_uri(const char *&requesttext, size_t &size) {
    auto requesturi = parse_till_space(requesttext, size);
    path = requesturi;
}

// Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
// We will allow CR
void request_header::parse_request_line(const char *&requesttext, size_t &size) {
    parse_method(requesttext, size);
    parse_skip_one(requesttext, size);
    parse_request_uri(requesttext, size);
    parse_skip_one(requesttext, size);
    parse_version<true>(requesttext, size);
}

void request::parse(const char *&requesttext, size_t &size) {
    parse_request_line(requesttext, size);
    if (size) {
        parse_fields(requesttext, size);
        if (size) body = std::string_view { requesttext, size };
    }
}

request::request(const std::string_view &text) {
    const char *requesttext = text.data();
    size_t size = text.size();
    parse(requesttext, size);
}

void response_header::parse_code(const char *&requesttext, size_t &size) {
    auto codetext = parse_till_space(requesttext, size);
    code = to_code_map(codetext);
    if (code == CODE::_0) throw MMS::http_parser_failed_t(requesttext, size);
}

void response_header::parse_response_line(const char *&requesttext, size_t &size) {
    parse_version<false>(requesttext, size);
    parse_skip_one(requesttext, size);
    parse_code(requesttext, size);
    parse_skip_one(requesttext, size);
    parse_till_CRLF(requesttext, size);
}

void response::parse(const char *&requesttext, size_t &size) {
    parse_response_line(requesttext, size);
    if (size) {
        parse_fields(requesttext, size);
        if (size) body = std::string_view { requesttext, size };
    }
}

response::response(const std::string_view &text) {
    const char *responsetext = text.data();
    size_t size = text.size();
    parse(responsetext, size);
}

} // namespace MMS::http