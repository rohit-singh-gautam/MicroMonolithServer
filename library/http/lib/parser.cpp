/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http/httpparser.h>
#include <format>

namespace MMS::http {

const std::unordered_map<std::string, FIELD> field_map = {
#define HTTP_FIELD_ENTRY(x, y) {y, FIELD::x},
    HTTP_FIELD_LIST
#undef HTTP_FIELD_ENTRY
};

const std::unordered_map<std::string, METHOD> method_map = {
#define HTTP_METHOD_ENTRY(x) {#x, METHOD::x},
    HTTP_METHOD_LIST
#undef HTTP_METHOD_ENTRY
};

const std::unordered_map<std::string, CODE> code_map_raw = {
#define HTTP_CODE_ENTRY(x, y) {#x, CODE::_##x},
    HTTP_CODE_LIST
#undef HTTP_CODE_ENTRY
};

const std::string header::empty { };

std::string request::to_string() {
    std::string ret {};
    ret += GetMethodStr();
    ret += " ";
    ret += GetPath();
    ret += " ";
    ret += MMS::http::to_string(version);
    ret += "\r\n";
    for(auto fieldentry: fields) {
        auto field = fieldentry.first;
        auto value = fieldentry.second;
        ret += MMS::http::to_string(field);
        ret += ": ";
        ret += value;
        ret += "\r\n";
    }
    ret += "\r\n";
    if ( !body.empty() ) {
        ret += body;
    }
    return ret;
}

std::string response::to_string() const {
    std::string ret = std::format(
        "{} {} {}\r\n",
        MMS::http::to_string(version),
        static_cast<int>(code),
        MMS::http::to_string(code)
    );

    for(auto fieldentry: fields) {
        auto field = fieldentry.first;
        auto value = fieldentry.second;
        ret += MMS::http::to_string(field);
        ret += ": ";
        ret += value;
        ret += "\r\n";
    }
    ret += "\r\n";
    if ( !body.empty() ) {
        ret += body;
    }
    return ret;
}

std::string parse_till_space(const ConstFullStream &stream) {
    auto start = stream.curr();
    while(*stream != ' ' && stream.remaining_buffer()) {
        ++stream;
    }

    if (start == stream.curr()) {
        throw MMS::http_parser_failed_t(stream);
    }

    return { reinterpret_cast<const char *>(start), static_cast<size_t>(stream.curr() - start)};
}

std::string parse_till_colon(const ConstFullStream &stream) {
    while(*stream == ' ' && stream.remaining_buffer()) {
        ++stream;
    }
    auto start = stream.curr();
    while(*stream != ':' && *stream != ' ' && stream.remaining_buffer()) {
        ++stream;
    }
    if (start == stream.curr()) {
        throw MMS::http_parser_failed_t(stream);
    }
    std::string ret = { reinterpret_cast<const char *>(start), static_cast<size_t>(stream.curr() - start) };

    while(*stream != ':' && stream.remaining_buffer()) {
        ++stream;
    }

    if (stream.remaining_buffer()) {
        // Skip colon
        ++stream;
    }

    return ret;
}

std::string parse_till_CRLF(const ConstFullStream &stream) {
    while(*stream == ' ' && stream.remaining_buffer()) {
        ++stream;
    }
    auto start = stream.curr();
    auto end = stream.curr();
    while(stream.remaining_buffer()) {
        if (*stream == '\r' || *stream == '\n') break;
        if (*(stream.curr() - 1) != ' ') end = stream.curr();
        ++stream;
    }
    if (*(stream.curr() - 1) != ' ') end = stream.curr();

    std::string ret = { reinterpret_cast<const char *>(start), static_cast<size_t>(end - start) };
    
    if (stream.remaining_buffer()) {
        if (*stream == '\r') {
            ++stream;
            if (stream.full() || *stream != '\n') throw MMS::http_parser_failed_t(stream);
            ++stream;
        } else if(*stream == '\n') {
            ++stream;
        }
    }

    return ret;
}

void parse_skip_one(const ConstFullStream &stream) {
    if (stream.full()) throw MMS::http_parser_failed_t(stream);
    ++stream;
}


bool parse_check_CRLF(const ConstFullStream &stream) {
    if (*stream == '\r') {
        ++stream;
        if (stream.full() || *stream != '\n') throw MMS::http_parser_failed_t(stream);
        ++stream;
        return true;
    } else if(*stream == '\n') {
        ++stream;
        return true;
    }
    return false;
}

template <bool crlf_end>
void header::parse_version(const ConstFullStream &stream) {
    const auto versiontext = crlf_end ? parse_till_CRLF(stream) : parse_till_space(stream);
    version = to_version(versiontext);
    if (version == VERSION::VER_UNKNOWN) throw MMS::http_parser_failed_t(stream);
}

void header::parse_fields(const ConstFullStream &stream) {
    while(true) {
        auto fieldtext = parse_till_colon(stream);
        auto value = parse_till_CRLF(stream);
        auto field = to_field(fieldtext);
        // When field is not present in enumeration it will be ignored.
        if (field != FIELD::IGNORE_THIS) fields[field] = std::move(value);
        if (stream.full() || parse_check_CRLF(stream)) break;
    }
}

void header::parse_method(const ConstFullStream &stream) {
    auto methodtext = parse_till_space(stream);
    SetMethod(std::move(methodtext));
    if (GetMethod() == METHOD::IGNORE_THIS) throw MMS::http_parser_failed_t(stream);
}

void header::parse_request_uri(const ConstFullStream &stream) {
    auto requesturi = parse_till_space(stream);
    SetPath(std::move(requesturi));
}

// Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
// We will allow CR
void header::parse_request_line(const ConstFullStream &stream) {
    parse_method(stream);
    parse_skip_one(stream);
    parse_request_uri(stream);
    parse_skip_one(stream);
    parse_version<true>(stream);
}

void request::parse(const ConstFullStream &stream) {
    parse_request_line(stream);
    if (stream.remaining_buffer()) {
        parse_fields(stream);
        if (stream.remaining_buffer()) body = std::string { reinterpret_cast<const char *>(stream.curr()), stream.remaining_buffer() };
    }
}

request::request(const ConstFullStream &stream) {
    parse(stream);
}

response request::CreateErrorResponse(CODE code, const std::string &errortext, const std::string &servername) const {
    response res = response::CreateBasicResponse(code);
    res.fields.emplace(FIELD::Connection, std::string { "close" });
    res.add_field(FIELD::Server, servername);
    
    auto accepttype = GetField(FIELD::Accept);
    bool accepthtml = accepttype.find("text/html") != std::string::npos;
    if (!accepthtml) {
        if (accepttype.find("application/json") != std::string::npos) {
            res.body = std::format(
                "{{\"error\": \"{}\", \"details\": \"{}\"}}",
                MMS::http::to_string(code), errortext
            );
            res.add_field(FIELD::Content_Type, std::string { "application/json" });
        } else accepthtml = true;
    }

    if (accepthtml) {
        res.body = std::format(
            "<html>"
            "<head><title>{0}</title></head>"
            "<h1>{2}</h1>"
            "<h2>{0}</h2>"
            "<pre>{1}</pre>"
            "</html>", 
            MMS::http::to_string(code), errortext, servername
        );
        res.add_field(FIELD::Content_Type, std::string { "text/html" });
    }
    res.UpdateContentLength();
    return res;
}

response response::CreateErrorResponse(CODE code, const std::string &errortext, const std::string &servername) {
    auto res = CreateBasicResponse(code);
    res.add_field(FIELD::Connection, std::string { "Close" });
    res.add_field(FIELD::Server, servername);
    res.add_field(FIELD::Content_Type, std::string { "text/html" });
    res.body = std::format(
        "<html>"
        "<head><title>{0}</title></head>"
        "<h1>Micro Monolith Server</h1>"
        "<h2>{0}</h2>"
        "<pre>{1}</pre>"
        "</html>", 
        MMS::http::to_string(code), errortext
    );
    res.UpdateContentLength();
    return res;
}

void response_header::parse_code(const ConstFullStream &stream) {
    auto codetext = parse_till_space(stream);
    code = to_code_map(codetext);
    if (code == CODE::_0) throw MMS::http_parser_failed_t(stream);
}

void response_header::parse_response_line(const ConstFullStream &stream) {
    parse_version<false>(stream);
    parse_skip_one(stream);
    parse_code(stream);
    parse_skip_one(stream);
    parse_till_CRLF(stream);
}

void response::parse(const ConstFullStream &stream) {
    parse_response_line(stream);
    if (stream.remaining_buffer()) {
        parse_fields(stream);
        if (stream.remaining_buffer()) body = std::string { reinterpret_cast<const char *>(stream.curr()), stream.remaining_buffer() };
    }
}

response::response(const ConstFullStream &stream) {
    parse(stream);
}

response response::CreateBasicResponse(CODE code) {
    constexpr uint64_t max_date_string_size = 92;
    std::time_t now_time = std::time(0);   // get time now
    std::tm* now_tm = std::gmtime(&now_time);
    char date_str[max_date_string_size];
    strftime(date_str, max_date_string_size, "%a, %d %b %Y %H:%M:%S %Z", now_tm);
    
    response res { };
    res.version = VERSION::VER_1_1;
    res.code = code;
    res.fields.emplace(FIELD::Date, std::string { date_str });

    return res;
}

} // namespace MMS::http