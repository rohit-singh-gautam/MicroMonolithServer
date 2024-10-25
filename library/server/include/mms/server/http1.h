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
#include <mms/server/http.h>
#include <mms/net/base.h>
#include <mms/ds/prefixmap.h>
#include <http/httpparser.h>
#include <unordered_map>
#include <algorithm>

namespace MMS::server::http::v2 {
class protocol_t;
} // namespace MMS::server::http::v2

namespace MMS::server::http::v1 {

class protocol_t : public MMS::server::http::protocol_t {
    static constexpr size_t response_buffer_initial_size = 1_kb;
    MMS::http::request *current_request { nullptr };
    FullStreamAutoAlloc response_buffer {response_buffer_initial_size};
public:
    using MMS::server::http::protocol_t::protocol_t;
    protocol_t(const protocol_t &) = delete;
    protocol_t &operator=(const protocol_t &) = delete;
    using net::protocol_t::Write;
    void ProcessRead(const Stream &stream) override;
    void WriteError(const CODE code, const std::string &errortext) override;
    void Write(const CODE code, const Stream &bodystream, std::vector<std::pair<FIELD, std::string>> &fields) override;
    void Write(const CODE code, std::vector<std::pair<FIELD, std::string>> &fields) override;
};

class creator_t : public MMS::server::http::creator_t {
public:
    using MMS::server::http::creator_t::creator_t;
    creator_t(const creator_t &) = delete;
    creator_t &operator=(const creator_t &) = delete;

    net::protocol_t *create_protocol(int logid, const std::string_view &protoname) override;
};

} // namespace MMS::server::http::v1

namespace MMS::server::http::v2 {
class protocol_t;
} // namespace MMS::server::http::v2

namespace MMS::client::http::v1 {

class protocol_t : public MMS::client::http::protocol_t {
    static constexpr size_t request_buffer_initial_size = 1_kb;
    FullStreamAutoAlloc request_buffer {request_buffer_initial_size};

public:
    using MMS::client::http::protocol_t::protocol_t;
    protocol_t(const protocol_t &) = delete;
    protocol_t &operator=(const protocol_t &) = delete;
    using net::protocol_t::Write;
    void ProcessRead(const Stream &stream) override;
    void Write(const METHOD method, const std::string &uri) override;
    void Write(const METHOD method, const std::string &uri, std::vector<std::pair<FIELD, std::string>> &fields) override;
    void Write(const METHOD method, const std::string &uri, const Stream &bodystream, std::vector<std::pair<FIELD, std::string>> &fields) override;
};

} // namespace MMS::client::http::v1
