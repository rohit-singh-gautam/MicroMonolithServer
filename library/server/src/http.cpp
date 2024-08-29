/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/http.h>
#include <format>

namespace MMS::server::http {

void protocol_t::ProcessRead(const uint8_t *buffer, const size_t size) {
    try {
        MMS::http::request request { {reinterpret_cast<const char *>(buffer), size}};
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
        WriteError(CODE::_400, parser_failed.to_string(reinterpret_cast<const char *>(buffer)));
    }
}

} // namespace MMS::server::http