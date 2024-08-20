/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/http.h>
#include <http/httpparser.h>
#include <format>

namespace MMS::server::http {

void protocol_t::ProcessRead(const uint8_t *buffer, const size_t size, listener::writer_t &writer) {
    try {
        MMS::http::request request { {reinterpret_cast<const char *>(buffer), size}};
        auto &handler = configuration.handlermap.search(request.GetPath());
        if (handler == nullptr) {
            auto response = request.CreateErrorResponse(MMS::http::CODE::_404, std::format("Path {} not found", request.GetPath()));
            writer.Write(response.to_string());
        }
        else {
            handler->ProcessRead(request, writer);
        }
    }
    catch(http_parser_failed_t &parser_failed) {
        auto response = MMS::http::response::CreateErrorResponse(MMS::http::CODE::_400, parser_failed.to_string(reinterpret_cast<const char *>(buffer)));
        writer.Write(response.to_string());        
    }
}

} // namespace MMS::server::http