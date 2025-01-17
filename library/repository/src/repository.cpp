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

#include <repository.h>
#include <mms/server/echo.h>
#include <mms/server/http.h>
#include <mms/server/http1.h>
#include <mms/server/cache.h>
#include <fstream>
#include <sstream>
#include <mms/listener.h>
#include <mms/net/udpserversimple.h>
#include <server/rest.h>

namespace MMS::repository {

std::string ReadStringFromFile(const std::filesystem::path &path) {
    if (!std::filesystem::is_regular_file(path)) {
        throw std::invalid_argument { "Not a valid file" };
    }
    std::ifstream filestream { path };
    std::stringstream buffer { };
    buffer << filestream.rdbuf();

    std::string content = buffer.str();
    filestream.close();
    return content;
}

bool Container::ReadProtocolConfiguration() {
    try {
        auto &json = ref["Protocols"];
        if (!json.IsObject() || json.empty()) return true;
        for(auto &member: json) {
            const auto &protoname = member.GetKey();
            auto &protoconf = member.GetValue();
            auto &type = protoconf["Type"];
            auto &type_name = type.GetString();
            if (type_name == "ECHO") {
                auto proto = new MMS::server::echocreator_t { };
                protocols.emplace(protoname, proto);
            } else if (type_name == "HTTP") {
                const auto &confname = protoconf["Configuration"].GetString();
                if (!httpconfigurations.contains(confname)) {
                    std::cerr << "Not able to find configuration with name "  << confname << std::endl;
                    return false;
                }

                auto &httpconf = httpconfigurations[confname];
                auto proto = new MMS::server::http::v1::creator_t { httpconf.get() };
                protocols.emplace(protoname, proto);                
            } else {
                std::cerr << "Unknown protocol " << type_name << std::endl;
            }
        }
    } catch(rohit::json::Exception &e) {
        std::cerr << "Json error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool Container::ReadHTTPConfiguration() {
    try {
        auto &json = ref.Query("Configurations/HTTP");
        // If HTTP configuration is not present it will be ignored.
        if (!json.IsObject()) return true;
        for(auto &member: json) {
            auto &ConfName = member.GetKey();
            auto &confjson = member.GetValue();

            // Extracting Server Name and creation of configuration
            auto &ServerNameJson = confjson["Server Name"];
            if (!ServerNameJson.IsString()) {
                std::cerr << "Expected /configuration/http/<Conf Name>/Server Name parameter as string\n";
                return false;
            }
            auto &ServerName = ServerNameJson.GetString();
            auto ptrconf = new server::http::configuration_t  { ServerName };

            auto &MaxFrameSizeJson = confjson["Max Frame Size"];
            if (!MaxFrameSizeJson.IsError()) {
                ptrconf->max_frame_size = MaxFrameSizeJson.GetInt();
            }

            auto &httpversionjson = confjson["Version"];
            if (httpversionjson.IsObject()) {
                auto &http1json = confjson["HTTP1"];
                if (!http1json.IsError()) {
                    ptrconf->version.http1 = http1json.GetBool();
                }

                auto &http2json = confjson["HTTP2"];
                if (!http2json.IsError()) {
                    ptrconf->version.http2 = http2json.GetBool();
                }

                auto &http2prijson = confjson["HTTP2Pri"];
                if (!http2prijson.IsError()) {
                    ptrconf->version.http2pri = http2prijson.GetBool();
                }
            } else if (!httpversionjson.IsError()) {
                std::cerr << "Expected /configuration/http/<Conf Name>/Version as Object containing following optional configuraiton HTTP1, HTTP2, HTTP2Pri\n";
                return false;
            }

            // Read limits for HTTP 2
            auto &limitsjson = confjson["Limits"];
            if (limitsjson.IsObject()) {
                auto &MaxReadBuffer = limitsjson["Max Read Buffer"];
                if (!MaxReadBuffer.IsError()) ptrconf->limits.MaxReadBuffer = MaxReadBuffer.GetInt();
                auto &FrameSize = limitsjson["Frame Size"];
                if (!FrameSize.IsError()) {
                    ptrconf->limits.FrameSizeMin = FrameSize[0].GetInt();
                    ptrconf->limits.FrameSizeMax = FrameSize[1].GetInt();
                }
                auto &HeaderTableSize = limitsjson["Header Table Size"];
                if (!HeaderTableSize.IsError()) {
                    ptrconf->limits.HeaderTableSizeMin = HeaderTableSize[0].GetInt();
                    ptrconf->limits.HeaderTableSizeMax = HeaderTableSize[1].GetInt();
                }
                auto &ConcurrentStreams = limitsjson["Concurrent Streams"];
                if (!ConcurrentStreams.IsError()) {
                    ptrconf->limits.ConcurrentStreamsMin = ConcurrentStreams[0].GetInt();
                    ptrconf->limits.ConcurrentStreamsMax = ConcurrentStreams[1].GetInt();
                }
                auto &WindowSize = limitsjson["WINDOW SIZE"];
                if (!WindowSize.IsError()) {
                    ptrconf->limits.WindowsSizeMin = WindowSize[0].GetInt();
                    ptrconf->limits.WindowsSizeMax = WindowSize[1].GetInt();
                }
                auto &HeaderListSize = limitsjson["HEADER LIST SIZE"];
                if (!HeaderListSize.IsError()) {
                    ptrconf->limits.HeaderListSizeMin = HeaderListSize[0].GetInt();
                    ptrconf->limits.HeaderListSizeMax = HeaderListSize[1].GetInt();
                }

            } else if (!limitsjson.IsError()) {
                std::cerr << "Expected /configuration/http/<Conf Name>/Limits as Object containing following optional http limits configuraiton\n";
                return false;
            }

            // Extracting Hander parameter and adding it to configuration.
            auto &Handlers = confjson["Handlers"];
            if (!Handlers.IsObject()) {
                std::cerr << "Expecting atleast one HTTP handler\n";
                return false;
            }
            for(auto &handlermember: Handlers) {
                auto &ServerPath = handlermember.GetKey();
                auto &handlerjson = handlermember.GetValue();
                auto &HandlerTypejson = handlerjson["Type"];
                if (!HandlerTypejson.IsString()) {
                    std::cerr << "Expected handler type expected as string\n";
                    return false;
                }
                auto &HandlerType = HandlerTypejson.GetString();
                auto handleritr = handlers.find(HandlerType);
                if (handleritr == std::end(handlers)) {
                    std::cerr << "Unable to find handler name " << HandlerType << std::endl;
                    return false;
                }
                ptrconf->AddHandler(ServerPath, handleritr->second.get());
            }

            // Finally adding a configuration
            httpconfigurations.emplace(ConfName, ptrconf);
        }
    } catch(rohit::json::Exception &e) {
        std::cerr << "Json error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool Container::ReadSSLConfiguration() {
    try {
        auto &json = ref.Query("Configurations/SSL");
        if (!json.IsObject()) return true;
        for(auto &confjsonitr: json) {
            auto &confname = confjsonitr.GetKey();
            auto &confjson = confjsonitr.GetValue();
            auto cert = confjson["Certificate"].GetString();
            auto prikey = confjson["Private Key"].GetString();
            auto ssl_common = new MMS::net::ssl::common { cert.c_str(), prikey.c_str() };
            SSLConfigurations.emplace(confname, ssl_common);
        }
    } catch(rohit::json::Exception &e) {
        std::cerr << "Json error: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool Container::ReadServerConfiguration() {
    try {
        auto &json = ref["Servers"];
        if (!json.IsObject() || json.empty()) {
            std::cerr << "Atleast one server configuration is expected\n";
            return false;
        }

        for(auto &memberjson: json) {
            auto &server_name = memberjson.GetKey();
            auto &serverjson = memberjson.GetValue();
            auto &protocol_name = serverjson["Protocol"].GetString();
            
            auto protoitr = protocols.find(protocol_name);
            if (protoitr == std::end(protocols)) {
                std::cerr << "Unable to find protocol with name " << protocol_name << std::endl;
                return false;
            }

            auto &proto = *protoitr->second;

            auto &port = serverjson["Port"].GetInt();
            auto &transport_name = serverjson["Transport"].GetString();

            listener::processor_t *server { nullptr };

            if (transport_name == "TCPSSL") {
                auto &ssl_conf_name = serverjson["SSL"].GetString();
                auto ssl_conf_itr = SSLConfigurations.find(ssl_conf_name);
                if (ssl_conf_itr == std::end(SSLConfigurations)) {
                    std::cerr << "SSL Configuration must be present\n";
                    return false;
                }
                auto sslconf = ssl_conf_itr->second.get();
                server = new net::tcp::ssl::server_t { port, proto, listener, sslconf };
            } else if (transport_name == "TCP") {
                server = new net::tcp::server_t { port, proto, listener };
            } else if (transport_name == "UDP") {
                server = new net::udp::server_t { port, proto, listener };
            } else {
                std::cerr << "Unknown transport name " << transport_name << std::endl;
            }

            Servers.emplace(server_name, server);
        }
    } catch(rohit::json::Exception &e) {
        std::cerr << "Json error: " << e.what() << std::endl;
        return false;
    } catch(MMS::bind_fail_t &e) {
        std::cerr << e.to_string() << std::endl;
        std::cerr << "Run 'sudo ./netmem.sh" << std::endl;
        return false;
    }

    return true;
}

bool Container::ReadHandlerConfiguration() {
    try {
        auto &json = ref["Handlers"];
        if (!json.IsObject()) {
            return true;
        }
        for(auto &memberjson: json) {
            auto &handlername = memberjson.GetKey();
            auto &handlerjson = memberjson.GetValue();
            auto &type_name = handlerjson["Type"].GetString();
            if (type_name == "File") {
                auto &filepath = handlerjson["File Path"].GetString();
                auto &defaultfile = handlerjson["Default File List"].GetString();
                auto defaultfileitr = string_lists.find(defaultfile);
                if (defaultfileitr == std::end(string_lists)) {
                    std::cerr << "Unable to find default file list for file handler" << std::endl; 
                    return false;
                }
                auto &mimemap = handlerjson["Mime Map"].GetString();
                auto mimemapitr = string_maps.find(mimemap);
                if (mimemapitr == std::end(string_maps)) {
                    std::cerr << "Unable to find map by name " << mimemap << std::endl;
                    return false;
                }
                auto handlerptr = new MMS::server::httpfilehandler { filecache, filepath, defaultfileitr->second, mimemapitr->second };
                handlers.emplace(handlername, handlerptr);
            } else if (type_name == "REST") {
                // TODO: Implement RESTCache
                auto &service = handlerjson["Services"].GetString();
                auto serviceitr = services.find(service);
                if (serviceitr == std::end(services)) {
                    std::cerr << "Unable to find service by name " << service << std::endl;
                    return false;
                }
                auto handlerptr = new MMS::server::rest::handler { serviceitr->second.get() };
                handlers.emplace(handlername, handlerptr);
            }
        }
    } catch(rohit::json::Exception &e) {
        std::cerr << "Json error: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool Container::ReadServiceConfiguration()
{
    try {
        auto &json = ref["Services"];
        if (!json.IsObject()) {
            return true;
        }
        for(auto &memberjson: json) {
            auto &servicename = memberjson.GetKey();
            auto &servicejson = memberjson.GetValue();
            auto &type_name = servicejson["Type"].GetString();
            if (type_name == "Cache") {
                auto serviceptr = new MMS::server::cache { };
                services.emplace(servicename, serviceptr);
            }
        }
    } catch(rohit::json::Exception &e) {
        std::cerr << "Json error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool Container::ReadSystemConfiguration() {
    try {
        auto &json = ref["System"];
        if (json.IsError()) return true;
        auto &threadcountjson = json["Thread Count"];
        if (threadcountjson.IsError()) return true;
        auto threadcount = static_cast<size_t>(threadcountjson.GetInt());
        listener->SetThreadCount(threadcount);

        
        streamlimit_t limits { };
        auto &readlimit = json["Read Buffer"];
        if (!readlimit.IsError()) {
            if (!readlimit.IsArray()) {
                std::cerr << "Read Buffer must be an array with two elements\n";
                return false;
            }
            limits.MinReadBuffer = readlimit[0].GetInt();
            limits.MaxReadBuffer = readlimit[1].GetInt();
            listener->SetReadBufferLimits(limits);
        }

    } catch(rohit::json::Exception &e) {
        std::cerr << "Json error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool Container::ReadData() {
    try {
        auto &json = ref["Data"];
        // Data is optional
        if (json.IsError()) return true;
        auto &listjson = json["List"];
        for(auto &memberlist: listjson) {
            auto &name = memberlist.GetKey();
            std::vector<std::string> list = memberlist.GetValue().GetStringVector(true);
            string_lists.emplace(name, std::move(list));
        }

        auto &mapjson = json["Map"];
        for(auto &membermap: mapjson) {
            auto &name = membermap.GetKey();
            std::unordered_map<std::string, std::string> map = membermap.GetValue().GetStringMap(true);
            string_maps.emplace(name, std::move(map));
        }
    } catch(rohit::json::Exception &e) {
        std::cerr << "Json error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool Container::ReadConfigurations() {
    if (!ReadData()) {
        std::cerr << "Unable to read Data types\n";
        return false;
    }
    if (!ReadServiceConfiguration()) {
        std::cerr << "Unable to read Service Configuration\n";
        return false;
    }
    if (!ReadHandlerConfiguration()) {
        std::cerr << "Unable to read Handler Configuration\n";
        return false;
    }
    if (!ReadSystemConfiguration()) {
        std::cerr << "Unable to read System Configuration\n";
        return false;
    }

    if (!ReadSSLConfiguration()) {
        std::cerr << "Unable to read SSL configurations\n";
        return false;
    }

    if (!ReadHTTPConfiguration()) {
        std::cerr << "Unable to read HTTP configurations\n";
        return false;
    }

    if (!ReadProtocolConfiguration()) {
        std::cerr << "Unable to read HTTP Protocol configurations\n";
        return false;
    }

    if (!ReadServerConfiguration()) {
        std::cerr << "Unable to read Server configurations\n";
        return false;
    }

    return true;
}

void Container::AddServerToListener() {
    for(auto &server: Servers) {
        listener->add(server.second.get());
    }
}

} // namespace MMS::repository