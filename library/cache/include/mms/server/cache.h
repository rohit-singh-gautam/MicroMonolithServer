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
#include <mms/server/ServiceBase.h>
#include <mms/server/http.h>
#include <unordered_map>
#include <map>
#include <memory>
#include <vector>


namespace MMS::server {

class cache : public ServiceBase {
    std::unordered_map<std::string, std::map<std::string, std::string>> string_maps { };

public:
    CODE CallAPI(const METHOD method, const std::string &api, ResponseType &type, const std::string &requestdata, Stream &responsedata) override;
};

} // namespace MMS::server

namespace MMS::client
{

using response_function = std::function<void(err_t, const std::string &)>;

class cache : public ClientBase {
    std::unordered_map<std::string, std::map<std::string, const std::string>> string_maps { };
    response_function responseFn;

public:
    void Response(const CODE code, ResponseType &type, const Stream &responsedata) override;

    err_t CreateMap(const std::string &mapname, response_function responseFn);
    err_t DeleteMap(const std::string &mapname, response_function responseFn);
    err_t AddMapEntry(const std::string &mapname, const std::string &key, const std::string &value, response_function responseFn);
    err_t GetMapEntry(const std::string &mapname, const std::string &key, response_function responseFn);
    err_t DeleteMapEntry(const std::string &mapname, const std::string &key, response_function responseFn);
};
    
} // namespace MMS::client

