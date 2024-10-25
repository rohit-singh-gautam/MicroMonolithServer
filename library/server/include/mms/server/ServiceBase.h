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