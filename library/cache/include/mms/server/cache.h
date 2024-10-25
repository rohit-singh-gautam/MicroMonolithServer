/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

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

