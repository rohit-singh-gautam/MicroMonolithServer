/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <repository.h>
#include <mms/server/echo.h>
#include <mms/server/http.h>
#include <fstream>
#include <sstream>
#include <mms/listener.h>

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
            } else if (type_name == "HTTPv1") {
                const auto &confname = protoconf["Configuration"].GetString();
                if (!httpconfigurations.contains(confname)) {
                    std::cerr << "Not able to find configuration with name "  << confname << std::endl;
                    return false;
                }

                auto &httpconf = httpconfigurations[confname];
                auto proto = new MMS::server::http::creator_t { *httpconf };
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
                if (HandlerType == "File Handler") {
                    auto &HandlerPathJson = handlerjson["File Path"];
                    if (!HandlerPathJson.IsString()) {
                        std::cerr << "Expected File Path as string\n";
                        return false;
                    }
                    auto &HandlerPath = HandlerPathJson.GetString();
                    std::unique_ptr<MMS::server::http::handler_t> handlerptr { new MMS::server::httpfilehandler { filecache, HandlerPath, *ptrconf } };
                    ptrconf->AddHandler(ServerPath, std::move(handlerptr));
                }
            }
            
            // Adding Dafault File that will be servered if server path is requested
            auto &DefaultFileListJson = confjson["Default File"];
            if (!DefaultFileListJson.IsArray()) {
                std::cerr << "Expected Default File list\n";
                return false;
            }

            for(auto &FileNameJson: DefaultFileListJson) {
                if (!FileNameJson.IsString()) {
                    std::cerr << "Expected filename as string in Default file list\n";
                    return false;
                }
                auto &FileName = FileNameJson.GetString();
                ptrconf->defaultlist.push_back(FileName);
            }

            // MIMEMAP
            auto &MIMEMAPJson = confjson["MIMEMAP"];
            if (!MIMEMAPJson.IsObject()) {
                std::cerr << "MIMEMAP required as object\n";
                return false;
            }
            for(auto &MimeMapEntry: MIMEMAPJson) {
                auto &ext = MimeMapEntry.GetKey();
                auto &MIMEJson = MimeMapEntry.GetValue();
                if (!MIMEJson.IsString()) {
                    std::cerr << "MIME is expected as string\n";
                    return false;
                }
                auto &mime = MIMEJson.GetString();
                ptrconf->mimemap.emplace(ext, mime);
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
            auto &sslconf = *ssl_conf_itr->second;
            server = new net::tcp::ssl::server_t { port, sslconf, proto, listner };
        } else if (transport_name == "TCP") {
            server = new net::tcp::server_t { port, proto, listner };
        } else {
            std::cerr << "Unknown transport name " << transport_name << std::endl;
        }

        Servers.emplace(server_name, server);
    }
    } catch(rohit::json::Exception &e) {
        std::cerr << "Json error: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool Container::ReadConfigurations() {
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
        listner->add(server.second.get());
    }
}

} // namespace MMS::repository