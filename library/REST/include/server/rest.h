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
#include <mms/server/ServiceBase.h>

namespace MMS::server::rest {

class handler : public http::handler_t {
    ServiceBase *impl;

public:
    handler(ServiceBase *service) : impl { service } { }
    handler(const handler &) = delete;
    handler &operator=(const handler &) = delete;

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