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
                responsedata.Write("{\"name\":\"", name, "\"}");
                if (!string_maps.contains(name)) {
                    string_maps.insert(std::move(name), SplitOrderList<std::string, std::string> { });
                    return CODE::Created;
                } else return CODE::OK;
            }

            case METHOD::DELETE: {
                // TODO: Check for validity
                auto &name = ref["name"].GetString();
                auto &keyjson = ref["key"];
                if (keyjson.IsError()) {
                    responsedata.Write("{\"name\":\"", name, "\"}");
                    if(string_maps.remove(name)) {
                        return CODE::OK;
                    } else {
                        return CODE::No_Content;
                    }
                } else {
                    auto mapopt = string_maps.find(name);
                    if (!mapopt) {
                        return CODE::No_Content;
                    }
                    auto &mapval = mapopt.value().get();
                    const auto key = keyjson.GetString();
                    responsedata.Write("{\"name\":\"", name, "\",\"key\":\"", key, "\"}");
                    if (mapval.remove(key)) {
                        return CODE::OK;
                    } else {
                        return CODE::No_Content;
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
                auto curropt = string_maps.find(name);
                if (!curropt) return CODE::Not_Found;
                auto &currmap = curropt.value().get();
                auto dataopt = currmap.find(key);
                // As json ref is not being use beyond this point
                // std::move for value will avoid one copy
                // Helpful for large data.
                responsedata.Write("{\"name\":\"", name, "\",\"key\":\"", key, "\"}");
                if (!dataopt) {
                    currmap.insert(key, std::move(value));
                    return CODE::Created;
                } else {
                    auto &dataval = dataopt.value().get();
                    std::swap(dataval, value);
                    return CODE::OK;
                }
                break;
            }
            
            case METHOD::GET: {
                // TODO:Check for validity
                const auto &name = ref["name"].GetString();
                auto mapopt = string_maps.find(name);
                if (mapopt) {
                    const auto &key = ref["key"].GetString();
                    auto &mapval = mapopt.value().get();
                    auto dataopt = mapval.find(key);
                    if (dataopt) {
                        responsedata.Write("{\"name\":\"", name, "\",\"key\":\"", key, "\",\"value\":\"", dataopt.value().get(), "\"}");
                        return CODE::OK;
                    } else return CODE::Not_Found;
                } else return CODE::Not_Found;
            }

            default:
                break;
                
        }
    }
    
    return CODE::Not_Found;
}

} // namespace MMS::server

namespace MMS::client {

void cache::Response(const CODE code, ResponseType &type, const Stream &responsedata) {
    assert(type == ResponseType::JSON);
    switch(code) {
    case CODE::OK: {
        auto ref = rohit::json::Parse(responsedata.curr(), responsedata.end());
        auto &keyjson = ref["key"];
        if (keyjson.IsError()) {
            responseFn(err_t::SUCCESS, {});
        } else {
            responseFn(err_t::SUCCESS, ref["value"].GetString());
        }
        break;
    }

    case CODE::Created:
        responseFn(err_t::SUCCESS, {});
        break;

    case CODE::No_Content:
    case CODE::Not_Found:
        responseFn(err_t::NOT_FOUND, {});
        break;

    default:
        responseFn(err_t::FAILURE, {});
        break;
    }
}

} // namespace MMS::client