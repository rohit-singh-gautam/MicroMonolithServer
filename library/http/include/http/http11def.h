/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string_view>

namespace MMS::http::version11 {
/*
    https://www.rfc-editor.org/rfc/rfc2616
    HTTP/1.1 206 Partial content
    Date: Wed, 15 Nov 1995 06:25:24 GMT
    Last-Modified: Wed, 15 Nov 1995 04:58:08 GMT
    Content-Range: bytes 21010-47021/47022
    Content-Length: 26012
    Content-Type: image/gif


*/

constexpr std::string_view HTTP_Version { "HTTP/1.1" };


} // namespace MMS::http::version11