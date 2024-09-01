/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

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

class creator_t;
class protocol_t : public MMS::server::http::protocol_t {
    MMS::http::request *current_request { nullptr };
    MMS::server::http::v2::protocol_t *http2_upgrade { nullptr };

public:
    ~protocol_t();
    using MMS::server::http::protocol_t::protocol_t;
    protocol_t(const protocol_t &) = delete;
    protocol_t &operator=(const protocol_t &) = delete;
    using net::protocol_t::Write;
    void ProcessRead(const ConstStream &stream) override;
    void WriteError(const CODE code, const std::string &errortext) override;
    void Write(const CODE code, const ConstStream &bodystream, std::vector<std::pair<FIELD, std::string>> &fields) override;
    void Write(const CODE code, std::vector<std::pair<FIELD, std::string>> &fields) override;
};

class creator_t : public MMS::server::http::creator_t {
public:
    using MMS::server::http::creator_t::creator_t;
    creator_t(const creator_t &) = delete;
    creator_t &operator=(const creator_t &) = delete;

    net::protocol_t *create_protocol(int logid, const std::string_view &protoname) override;
};

} // namespace MMS::server::http
