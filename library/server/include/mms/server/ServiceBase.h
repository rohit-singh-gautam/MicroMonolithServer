/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/base/stream.h>
#include <http/httpdef.h>

namespace MMS::server {
using MMS::http::METHOD;
using MMS::http::CODE;

enum class ResponseType {
    Empty,
    JSON,
    XML,
    CSV,
    Binary
};


class ServiceBase {
public:
    virtual ~ServiceBase() = default;
    virtual CODE CallAPI(const METHOD method, const std::string &api, ResponseType &type, const std::string &requestdata, Stream &responsedata) = 0;
};


} // namespace MMS::server