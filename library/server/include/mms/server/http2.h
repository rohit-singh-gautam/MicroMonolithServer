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
#include <http/http2.h>
#include <mms/net/base.h>
#include <mms/ds/prefixmap.h>
#include <http/httpparser.h>
#include <unordered_map>

namespace MMS::server::http::v2 {
using MMS::http::CODE;
using MMS::http::FIELD;
using MMS::http::METHOD;
using MMS::http::request;
using MMS::http::response;

class protocol_t : public MMS::server::http::protocol_t{
    static constexpr size_t response_buffer_initial_size = 1_kb;
    bool first_frame { true };
    bool settings_responded { false };
    MMS::http::v2::header_request *header_request { nullptr };
    FullStreamAutoAlloc response_buffer { response_buffer_initial_size };

    MMS::http::hpack::dynamic_table_t dynamic_table { };
    MMS::http::v2::settings_store peer_settings { };

public:
    using MMS::server::http::protocol_t::protocol_t;
    using net::protocol_t::Write;
    protocol_t(const protocol_t &) = delete;
    protocol_t &operator=(const protocol_t &) = delete;

    void AddSettingResponse();
    void AddBase64Settings(const std::string &settings);
    void Upgrade(MMS::http::request &&);
    void ProcessRequest();

    void ProcessRead(const Stream &stream) override;
    void WriteError(const CODE code, const std::string &errortext) override;
    void Write(const CODE code, const Stream &bodystream, std::vector<std::pair<FIELD, std::string>> &fields) override;
    void Write(const CODE code, std::vector<std::pair<FIELD, std::string>> &fields) override;
    void FinalizeWrite(); // this is required for HTTP v2
};

} // namespace MMS::server::http
