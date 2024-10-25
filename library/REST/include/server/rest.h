/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/server/http.h>
#include <mms/server/ServiceBase.h>

namespace MMS::server::rest {

class handler : public http::handler_t {
    ServiceBase *impl;

public:
    void ProcessRead(const MMS::http::request &request, const std::string &relative_path, http::protocol_t *writer) override;

    constexpr const std::vector<http::METHOD> &GetSupportedMethod() override {
        static const std::vector<http::METHOD> supported_methods {
            http::METHOD::GET,
            http::METHOD::PUT,
            http::METHOD::POST,
            http::METHOD::DELETE
        };
        return supported_methods;
    }

    constexpr bool IsSupported(const http::METHOD method) override {
        return method == http::METHOD::GET || method == http::METHOD::PUT || method == http::METHOD::POST || method == http::METHOD::DELETE;
    }

};

} // namespace MMS::server::rest


namespace MMS::client::rest {

class cache : public MMS::client::http::handler_t {
    ClientBase *impl;
public:
    void ProcessRead(const MMS::http::response &response, MMS::client::http::protocol_t *writer) override;


};

} // namespace MMS::client::rest