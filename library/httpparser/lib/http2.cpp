/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http2.h>

namespace rohit::http::v2 {

std::ostream& operator<<(std::ostream& os, const header_request &header_request) {
    const http_header_request &http11request = header_request;

    os << "Stream Identifier: " << header_request.stream_identifier << std::endl
        << "Weight: " << (unsigned int)header_request.weight << std::endl;
    
    if (header_request.error != frame::error_t::NO_ERROR) {
        os << "Error: " << header_request.error << std::endl;
    }

    return os << http11request;
}

} // namespace rohit::http::v2