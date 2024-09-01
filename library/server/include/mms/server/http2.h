/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

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
using MMS::server::http::typecheck::fieldentrypair;

class protocol_t : public MMS::server::http::protocol_t{
    static constexpr size_t response_buffe_initial_size = 1_kb;
    bool first_frame { true };
    bool settings_responded { false };
    MMS::http::v2::header_request *header_request { nullptr };
    FullStreamAutoAlloc response_buffer { response_buffe_initial_size };

    MMS::http::v2::dynamic_table_t dynamic_table { };
    MMS::http::v2::settings_store peer_settings { };

public:
    using MMS::server::http::protocol_t::protocol_t;
    using net::protocol_t::Write;
    protocol_t(const protocol_t &) = delete;
    protocol_t &operator=(const protocol_t &) = delete;

    void AddSettingResponse();
    void AddBase64Settings(const std::string &settings);
    void Upgrade(MMS::http::request &&);

    void ProcessRead(const ConstStream &stream) override;
    void WriteError(const CODE code, const std::string &errortext) override;
    void Write(const CODE code, const ConstStream &bodystream, std::vector<std::pair<FIELD, std::string>> &fields) override;
    void Write(const CODE code, std::vector<std::pair<FIELD, std::string>> &fields) override;
    void FinalizeWrite(); // this is required for HTTP v2
};

} // namespace MMS::server::http
