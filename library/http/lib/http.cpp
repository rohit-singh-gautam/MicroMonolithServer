/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http/httpdef.h>
#include <ostream>

namespace MMS::http {

const std::unordered_map<std::string_view, header::FIELD> header::field_map = {
#define HTTP_FIELD_ENTRY(x, y) {y, header::FIELD::x},
    HTTP_FIELD_LIST
#undef HTTP_FIELD_ENTRY
};

const std::unordered_map<std::string_view, header_request::METHOD> header_request::method_map = {
#define HTTP_METHOD_ENTRY(x) {#x, header_request::METHOD::x},
    HTTP_METHOD_LIST
#undef HTTP_METHOD_ENTRY
};

} // namespace MMS::http