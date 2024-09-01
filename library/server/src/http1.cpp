/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/http.h>
#include <mms/server/http1.h>
#include <mms/server/http2.h>
#include <mms/net/tcpcommon.h>
#include <format>

namespace MMS::server::http::v1 {

protocol_t::~protocol_t() { if(http2_upgrade) delete http2_upgrade; }

void protocol_t::ProcessRead(const ConstStream &stream) {
    try {
        // Check for HTTP 2.0 Pri
        if (configuration->http2pri) {
            auto buffer = reinterpret_cast<const char *>(stream.curr());
            if (std::equal(std::begin(MMS::http::v2::connection_preface), std::end(MMS::http::v2::connection_preface), buffer)) {
                log<log_t::HTTP2_PRI_KNOWLEDGE>(GetFD());
                // Move to http2
                if (!http2_upgrade) {
                    http2_upgrade = new v2::protocol_t { configuration };
                    http2_upgrade->SetProcessor(processor);
                }
                stream += sizeof(MMS::http::v2::connection_preface);
                http2_upgrade->ProcessRead(stream);
                auto connection = dynamic_cast<MMS::net::tcp::connection_base_t *>(processor);
                assert(connection);
                /* -------------------IMP------------------------*/
                // This will free up current running class. Hence no class variable must be used beyond this point.
                auto temp_http2_upgrade = http2_upgrade;
                http2_upgrade = nullptr;
                connection->SetProtocol(temp_http2_upgrade);
                /* -------------------IMP------------------------*/
                return;
            }
        } else if (http2_upgrade) {
            // TODO:: Return protocol error
            log<log_t::HTTP2_UPGRADE_NO_PRI>(GetFD());
        }

        if (!configuration->http1) {
            WriteError(CODE::_505, { "HTTP1 not supported" });
            return;
        }
        MMS::http::request request { ConstFullStream { stream.curr(), stream.end() }};
        if (configuration->http2) {
            if (request.upgrade_version() == VERSION::VER_2) {
                log<log_t::HTTP2_UPGRADE>(GetFD());
                auto settingbase64 = request.GetField(FIELD::HTTP2_Settings);
                if (!settingbase64.empty())  {
                    // Time to move to HTTP2.
                    http2_upgrade = new v2::protocol_t { configuration };
                    http2_upgrade->AddBase64Settings(settingbase64);
                    http2_upgrade->SetProcessor(processor);
                    http2_upgrade->Upgrade(std::move(request));
                    return;
                }
            }
        }
        std::string newpath { };
        auto &handler = configuration->handlermap.search(request.GetPath(), newpath);
        if (handler == nullptr) {
            WriteError(CODE::_404,  std::format("Path {} not found", request.GetPath()));
        }
        else {
            current_request = &request;
            handler->ProcessRead(request, newpath, this);
            current_request = nullptr;
        }
    }
    catch(http_parser_failed_t &parser_failed) {
        WriteError(CODE::_400, parser_failed.to_string());
    }
}

void protocol_t::WriteError(const CODE code, const std::string &errortext) {
    if (current_request) {
        auto res = current_request->CreateErrorResponse(code, errortext, configuration->ServerName);
        Write(res.to_string());
    } else {
        auto res = response::CreateErrorResponse(code, errortext, configuration->ServerName);
        Write(res.to_string());
    }
}

void protocol_t::Write(const CODE code, const ConstStream &bodystream, std::vector<std::pair<FIELD, std::string>> &fields) {
    auto response = response::CreateBasicResponse(code);
    auto [bodybuffer, bodysize] = bodystream.GetRawCurrentBuffer();
    std::ranges::for_each(fields, [&response](const std::pair<FIELD, std::string> &field) { response.add_field(field); });
    response.add_field(MMS::http::FIELD::Server, configuration->ServerName);
    response.add_field(FIELD::Content_Length, bodysize);
    Write(
        listener::write_entry_const { response.to_string()},
        listener::write_entry_const { bodybuffer, bodysize });
}

} // namespace MMS::server::http