/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http/httpdef.h>

namespace MMS::http {
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

    void parse_method(header_request &request, const char *&requesttext, size_t &size) {
        auto methodtext = parse_till_space(requesttext, size);

        auto itr = request.method_map.find(methodtext);
        if (itr == std::end(request.method_map)) {
            throw MMS::http_parser_failed_t(requesttext, size);
        }

        request.method = itr->second;
    }

    void parse_request_uri(header_request &request, const char *&requesttext, size_t &size) {
        auto requesturi = parse_till_space(requesttext, size);
        request.path = requesturi;
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
    void parse_version(header_request &request, const char *&requesttext, size_t &size) {
        const auto versiontext = crlf_end ? parse_till_CRLF(requesttext, size) : parse_till_space(requesttext, size);

        if (versiontext.compare("HTTP/1.1") == 0) request.version = header::VERSION::VER_1_1;
        else if (versiontext.compare("HTTP/2.0") == 0) request.version = header::VERSION::VER_1_1;
        else throw MMS::http_parser_failed_t(requesttext, size);
    }

    // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
    // We will allow CR
    void parse_request_line(header_request &request, const char *&requesttext, size_t &size) {
        parse_method(request, requesttext, size);
        parse_skip_one(requesttext, size);
        parse_request_uri(request, requesttext, size);
        parse_skip_one(requesttext, size);
        parse_version<true>(request, requesttext, size);
    }

    void parse_request_fields(header_request &request, const char *&requesttext, size_t &size) {
        while(true) {
            auto field = parse_till_colon(requesttext, size);
            auto value = parse_till_CRLF(requesttext, size);
            auto itr = header::field_map.find(field);
            if (itr != std::end(header::field_map)) {
                // When field is not present in enumeration it will be ignored.
                request.fields[itr->second] = std::move(value);
            }
            if (!size || parse_check_CRLF(requesttext, size)) break;
        }
    }

size_t header_request::parse(const std::string_view &text) {
    const char *requesttext = text.data();
    auto start = requesttext;
    size_t size = text.size();
    parse_request_line(*this, requesttext, size);
    if (size) {
        parse_request_fields(*this, requesttext, size);
    }

    return static_cast<size_t>(requesttext - start);
}

} // namespace MMS::http