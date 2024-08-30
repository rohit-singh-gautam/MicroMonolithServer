/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/http.h>
#include <mms/server/http1.h>
#include <format>

namespace MMS::server::http::v1 {

void protocol_t::ProcessRead(const ConstStream &stream) {
    try {
        MMS::http::request request { ConstFullStream { stream.curr(), stream.end() }};
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

void protocol_t::Write(const CODE code, const char *bodybuffer, size_t bodysize, const std::vector<std::pair<FIELD, std::string>> &fields) {
    auto response = response::CreateBasicResponse(code);
    std::ranges::for_each(fields, [&response](const std::pair<FIELD, std::string> &field) { response.add_field(field); });
    response.add_field(MMS::http::FIELD::Server, configuration->ServerName);
    response.add_field(FIELD::Content_Length, bodysize);
    Write(
        listener::write_entry_const { response.to_string()},
        listener::write_entry_const { bodybuffer, bodysize });
}

} // namespace MMS::server::http