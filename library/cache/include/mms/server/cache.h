/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/server/ServiceBase.h>
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