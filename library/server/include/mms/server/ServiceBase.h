/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/base/stream.h>
#include <http/httpdef.h>
#include <unordered_map>

namespace MMS {
enum class ResponseType {
    Empty,
    JSON,
    XML,
    CSV,
    Binary
};

constexpr inline ResponseType GetResponseTypeFromMIME(const std::string &mime) {
    static std::unordered_map<std::string, ResponseType> responsetype_map {
        {"Empty", ResponseType::Empty},
        {"application/json", ResponseType::JSON},
        {"application/xml", ResponseType::XML},
        {"text/csv", ResponseType::CSV},
        {"application/mms-binary", ResponseType::Binary},
    };
    const auto itr = responsetype_map.find(mime);
    if (itr == std::end(responsetype_map)) {
        return ResponseType::Empty;
    } else {
        return itr->second;
    }
}

} // namespace MMS

namespace MMS::server {
using MMS::http::METHOD;
using MMS::http::CODE;


class ServiceBase {
public:
    virtual ~ServiceBase() = default;
    virtual CODE CallAPI(const METHOD method, const std::string &api, ResponseType &type, const std::string &requestdata, Stream &responsedata) = 0;
};


} // namespace MMS::server

namespace MMS::client {
using MMS::http::METHOD;
using MMS::http::CODE;

class ClientBase {
public:
    virtual ~ClientBase() = default;
    virtual void Response(const CODE code, ResponseType &type, const Stream &responsedata) = 0;
};

} // namespace MMS::client