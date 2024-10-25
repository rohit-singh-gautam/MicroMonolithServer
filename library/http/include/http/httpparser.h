//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2024  Rohit Jairaj Singh (rohit@singh.org.in)          //
//                                                                      //
// This program is free software: you can redistribute it and/or modify //
// it under the terms of the GNU General Public License as published by //
// the Free Software Foundation, either version 3 of the License, or    //
// (at your option) any later version.                                  //
//                                                                      //
// This program is distributed in the hope that it will be useful,      //
// but WITHOUT ANY WARRANTY; without even the implied warranty of       //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        //
// GNU General Public License for more details.                         //
//                                                                      //
// You should have received a copy of the GNU General Public License    //
// along with this program.  If not, see <https://www.gnu.org/licenses/>//
//////////////////////////////////////////////////////////////////////////

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
    void parse_version(const FullStream &);
    void parse_fields(const FullStream &);

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
    void parse_request_uri(const FullStream &);
    void parse_method(const FullStream &);
    void parse_request_line(const FullStream &);

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

    constexpr const auto GetPathBase() const { 
        auto &path = GetPath();
        auto pos = path.rfind('/');
        if (pos == std::string::npos) return path;
        else return path.substr(pos);
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
    request(const FullStream &);
    request(request &&other) : header { std::move(other) }, body { std::move(body) } { }

    constexpr const auto &GetBody() const { return body; }

    void parse(const FullStream &);

    response CreateErrorResponse(CODE code, const std::string &errortext, const std::string &servername) const;

    std::string to_string();
}; // class request

class response_header : public header {
protected:
    CODE code { };
    constexpr response_header() { }
    constexpr response_header(VERSION version) : header { version } { }

    void parse_code(const FullStream &);
    void parse_response_line(const FullStream &);
public:
    constexpr auto GetCode() const { return code; };
};

class response : public response_header {
    friend class request;
    std::string body { };

public:
    constexpr response() { }
    response(const FullStream &);

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

    void parse(const FullStream &);

    std::string to_string() const;
}; // response

struct http_version_t {
    bool http1 { true };
    bool http2 { true };
    bool http2pri { true };
};

struct http_limits_t {
    template <std::integral T>
    static T GetSize(T min, T max, T selected) {
        if (selected <= min) return min;
        if (selected >= max) return max;
        return selected;
    }
    uint32_t MaxReadBuffer { 4096 };
    uint32_t FrameSizeMin { 1024 };
    uint32_t FrameSizeMax { 65536 };
    uint32_t GetFrameSize(uint32_t value) const { return GetSize(FrameSizeMin, FrameSizeMax, value); }
    uint32_t HeaderTableSizeMin { 128 };
    uint32_t HeaderTableSizeMax { 4096 };
    uint32_t GetHeaderTableSize(uint32_t value) const { return GetSize(HeaderTableSizeMin, HeaderTableSizeMax, value); }
    uint32_t ConcurrentStreamsMin { 1 };
    uint32_t ConcurrentStreamsMax { 32 };
    uint32_t GetConcurrentStreams(uint32_t value) const { return GetSize(ConcurrentStreamsMin, ConcurrentStreamsMax, value); }
    uint32_t WindowsSizeMin { 16384 };
    uint32_t WindowsSizeMax { 1048576 };
    uint32_t GetWindowsSize(uint32_t value) const { return GetSize(WindowsSizeMin, WindowsSizeMax, value); }
    uint32_t HeaderListSizeMin { 8 };
    uint32_t HeaderListSizeMax { 128 };
    uint32_t GetHeaderListSize(uint32_t value) const { return GetSize(HeaderListSizeMin, HeaderListSizeMax, value); }
};

constexpr void WriteResponseLine(Stream &stream, const CODE code) { stream.Write("HTTP/1.1 ", std::to_string(static_cast<unsigned short>(code)), ' ', to_string(code), "\r\n"); }
constexpr void WriteRequestLine(Stream &stream, const METHOD method, const std::string &uri) { stream.Write(to_string(method), ' ', uri, " HTTP/1.1\r\n"); }
constexpr void WriteFieldLine(Stream &stream, const FIELD field, const std::string &value) { stream.Write(to_string(field), ": ", value, "\r\n"); }
constexpr void WriteFieldLine(Stream &stream, const std::vector<std::pair<FIELD, std::string>> &fields) { for(auto &field: fields) WriteFieldLine(stream, field.first, field.second); }
constexpr void WriteDateLine(Stream &stream) {
    constexpr uint64_t max_date_string_size = 92;
    std::time_t now_time = std::time(0);   // get time now
    std::tm* now_tm = std::gmtime(&now_time);
    char date_str[max_date_string_size];
    auto date_size = strftime(date_str, max_date_string_size, "%a, %d %b %Y %H:%M:%S %Z", now_tm);
    stream.Write(to_string(FIELD::Date), ": ");
    stream.Copy(date_str, date_size);
    stream.Write("\r\n");
}

} // namespace MMS::http
