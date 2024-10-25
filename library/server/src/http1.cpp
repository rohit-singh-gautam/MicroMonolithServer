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

net::protocol_t *creator_t::create_protocol(int fd, const std::string_view &protoname) { 
    if (protoname == "h2") {
        if (configuration->version.http2 || configuration->version.http2pri) {
            return new MMS::server::http::v2::protocol_t(configuration);
        }
        log<log_t::HTTP2_UNSUPPORTED>(fd);
    } else if (protoname == "http/1.1") {
        if (configuration->version.http1) {
            return new protocol_t { configuration };
        }

        log<log_t::HTTP1_UNSUPPORTED>(fd);
    } else if (protoname.empty()) {
        // This path is for non SSL
        return new protocol_t { configuration };
    } else {
        log<log_t::HTTP_UNSUPPORTED_PROTOCOL>(fd);
    }
    return nullptr;
}

void protocol_t::ProcessRead(const Stream &stream) {
    try {
        // Check for HTTP 2.0 Pri
        if (configuration->version.http2pri) {
            if (stream == std::string_view { MMS::http::v2::connection_preface, MMS::http::v2::connection_preface_size }) {
                log<log_t::HTTP2_PRI_KNOWLEDGE>(GetFD());
                // Move to http2
                auto http2_upgrade = new v2::protocol_t { configuration };
                http2_upgrade->SetProcessor(processor);
                http2_upgrade->ProcessRead(stream);
                auto connection = dynamic_cast<MMS::net::tcp::connection_base_t *>(processor);
                assert(connection);
                /* -------------------IMP------------------------*/
                // This will free up current running class. Hence no class variable must be used beyond this point.
                connection->SetProtocol(http2_upgrade);
                /* -------------------IMP------------------------*/
                return;
            }
        }

        if (!configuration->version.http1) {
            WriteError(CODE::_505, { "HTTP1 not supported" });
            return;
        }
        MMS::http::request request { make_const_fullstream(stream.curr(), stream.end())};
        if (configuration->version.http2) {
            if (request.upgrade_version() == VERSION::VER_2) {
                log<log_t::HTTP2_UPGRADE>(GetFD());
                auto settingbase64 = request.GetField(FIELD::HTTP2_Settings);
                if (!settingbase64.empty())  {
                    // Time to move to HTTP2.
                    auto http2_upgrade = new v2::protocol_t { configuration };
                    http2_upgrade->AddBase64Settings(settingbase64);
                    http2_upgrade->SetProcessor(processor);
                    http2_upgrade->Upgrade(std::move(request));
                    /* -------------------IMP------------------------*/
                    // This will free up current running class. Hence no class variable must be used beyond this point.
                    auto connection = dynamic_cast<MMS::net::tcp::connection_base_t *>(processor);
                    assert(connection);
                    connection->SetProtocol(http2_upgrade);
                    /* -------------------IMP------------------------*/
                    return;
                }
            }
        }
        std::string newpath { };
        auto &handler = configuration->handlermap.search(request.GetPath(), newpath);
        if (handler == nullptr) {
            WriteError(CODE::Not_Found,  std::format("Path {} not found", request.GetPath()));
        }
        else {
            current_request = &request;
            auto method = request.GetMethod();
            if (handler->IsSupported(method)) {
                handler->ProcessRead(request, newpath, this);
            } else if (method == METHOD::OPTIONS) {
                std::string optionsstr {};
                CreateSupportedMethodString(optionsstr, handler->GetSupportedMethod(), http::METHOD::PRI, http::METHOD::OPTIONS);
                std::vector<std::pair<FIELD, std::string>> fields {
                    std::pair<FIELD, std::string> {http::FIELD::Allow, optionsstr}
                };
                Write(http::CODE::No_Content, fields);
            } else {
                const std::string errstr = CreateSupportedMethodErrorString(method, handler->GetSupportedMethod(), http::METHOD::PRI, http::METHOD::OPTIONS);
                WriteError(http::CODE::Method_Not_Allowed, errstr);
            }
            current_request = nullptr;
        }
    }
    catch(exception_t &failed) {
        WriteError(CODE::Bad_Request, failed.to_string());
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

void protocol_t::Write(const CODE code, const Stream &bodystream, std::vector<std::pair<FIELD, std::string>> &fields) {
    MMS::http::WriteResponseLine(response_buffer, code);
    MMS::http::WriteDateLine(response_buffer);
    MMS::http::WriteFieldLine(response_buffer, FIELD::Server, configuration->ServerName);
    MMS::http::WriteFieldLine(response_buffer, fields);
    MMS::http::WriteFieldLine(response_buffer, FIELD::Content_Length, std::to_string(bodystream.remaining_buffer()));
    response_buffer.Write("\r\n");
    response_buffer.Copy(bodystream);
    auto writestream = response_buffer.ReturnOldAndAlloc(response_buffer_initial_size);
    FixedBuffer writebuffer { std::move(writestream) };
    WriteNoCopy(std::move(writebuffer));
}

void protocol_t::Write(const CODE code, std::vector<std::pair<FIELD, std::string>> &fields) {
    MMS::http::WriteResponseLine(response_buffer, code);
    MMS::http::WriteDateLine(response_buffer);
    MMS::http::WriteFieldLine(response_buffer, FIELD::Server, configuration->ServerName);
    MMS::http::WriteFieldLine(response_buffer, fields);
    auto writestream = response_buffer.ReturnOldAndAlloc(response_buffer_initial_size);
    FixedBuffer writebuffer { std::move(writestream) };
    WriteNoCopy(std::move(writebuffer));
}

} // namespace MMS::server::http

namespace MMS::client::http::v1 {
void protocol_t::Write(const METHOD method, const std::string &uri) { MMS::http::WriteRequestLine(request_buffer, method, uri); }
void protocol_t::Write(const METHOD method, const std::string &uri, std::vector<std::pair<FIELD, std::string>> &fields) {
    MMS::http::WriteRequestLine(request_buffer, method, uri);
    MMS::http::WriteFieldLine(request_buffer, fields);
    auto writestream = request_buffer.ReturnOldAndAlloc(request_buffer_initial_size);
    FixedBuffer writebuffer { std::move(writestream) };
    WriteNoCopy(std::move(writebuffer));
}
void protocol_t::Write(const METHOD method, const std::string &uri, const Stream &bodystream, std::vector<std::pair<FIELD, std::string>> &fields) {
    MMS::http::WriteRequestLine(request_buffer, method, uri);
    MMS::http::WriteFieldLine(request_buffer, fields);
    MMS::http::WriteFieldLine(request_buffer, FIELD::Content_Length, std::to_string(bodystream.remaining_buffer()));
    request_buffer.Write("\r\n");
    request_buffer.Copy(bodystream);
    auto writestream = request_buffer.ReturnOldAndAlloc(request_buffer_initial_size);
    FixedBuffer writebuffer { std::move(writestream) };
    WriteNoCopy(std::move(writebuffer));
}

void protocol_t::ProcessRead(const Stream &stream) {
    MMS::http::response response { make_const_fullstream(stream.curr(), stream.end()) };
    handler.ProcessRead(response, this);
}
} // MMS::client::http::v1