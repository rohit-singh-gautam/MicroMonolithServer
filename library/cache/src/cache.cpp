/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/cache.h>
#include <json.h>

namespace MMS::server {

CODE cache::CallAPI(const METHOD method, const std::string &api, ResponseType &type, const std::string &requestdata, Stream &responsedata)
{
    type = ResponseType::JSON;
    if (api == "v1/map") {
        auto ref = rohit::json::Parse(requestdata);
        if (ref.IsObject()) return CODE::Bad_Gateway;
        switch(method) {
            // Create is not required, but someone want to create map in advance
            // MMS will not stop him.
            case METHOD::POST: {
                // TODO: Check for validity
                auto &name = ref["name"].GetString();
                auto mapitr = string_maps.find(name);
                if (mapitr == string_maps.end()) {
                    string_maps.emplace(name, std::map<std::string, std::string> { });
                    return CODE::Created;
                } else return CODE::OK;
                break;
            }

            case METHOD::DELETE: {
                // TODO: Check for validity
                auto &name = ref["name"].GetString();
                auto mapitr = string_maps.find(name);
                if (mapitr == string_maps.end()) {
                    return CODE::No_Content;
                }
                auto &keyjson = ref["key"];
                if (keyjson.IsError()) {
                    string_maps.erase(mapitr);
                    return CODE::OK;
                } else {
                    const auto key = keyjson.GetString();
                    auto dataitr = mapitr->second.find(key);
                    if (dataitr == std::end(mapitr->second)) {
                        return CODE::No_Content;
                    } else {
                        mapitr->second.erase(dataitr);
                        return CODE::OK;
                    }
                }
                break;
            }

            case METHOD::PUT: {
                // TODO: Check for validity
                auto &name = ref["name"].GetString();
                auto &key = ref["key"].GetString();
                auto &value = ref["value"].GetString();
                
                // This will create map if it does not exists.
                auto &currmap = string_maps[name];
                auto dataitr = currmap.find(key);
                // As json ref is not being use beyond this point
                // std::move for value will avoid one copy
                // Helpful for large data.
                if (dataitr == std::end(currmap)) {
                    currmap.emplace(key, std::move(value));
                    return CODE::Created;
                } else {
                    dataitr->second = std::move(value);
                    return CODE::OK;
                }
                break;
            }
            
            case METHOD::GET: {
                // TODO:Check for validity
                const auto &name = ref["name"].GetString();
                auto mapitr = string_maps.find(name);
                if (mapitr != string_maps.end()) {
                    const auto &key = ref["key"].GetString();
                    auto dataitr = mapitr->second.find(key);
                    if (dataitr != std::end(mapitr->second)) {
                        responsedata.Write("{\"value\":", dataitr->second, '}');
                        return CODE::OK;
                    }
                }
            }

            default:
                break;
                
        }
    }
    
    return CODE::Not_Found;
}

} // namespace MMS::server