/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http/httpdef.h>
#include <ostream>

namespace MMS::http {

std::ostream& operator<<(std::ostream& os, const header::VERSION httpVersion) {
    switch(httpVersion) {
#define HTTP_VERSION_ENTRY(x, y) case header::VERSION::x: os << y; break;
    HTTP_VERSION_LIST
#undef HTTP_VERSION_ENTRY
    default:
        os << "Unknown version " << static_cast<int>(httpVersion);
        break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const header::FIELD requestField) {
    switch(requestField) {
#define HTTP_FIELD_ENTRY(x, y) case header::FIELD::x: os << y; break;
    HTTP_FIELD_LIST
#undef HTTP_FIELD_ENTRY
    default:
        os << "Unknown field";
        break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::pair<header::FIELD, std::string>& httpFieldPair) {
    return os << httpFieldPair.first << ": " << httpFieldPair.second;
}

std::ostream& operator<<(std::ostream& os, const header::fields_t& httpFields) {
    for(auto httpFieldPair: httpFields) {
        os << httpFieldPair << "\n";
    }
    return os;
}

const std::unordered_map<std::string, header::FIELD> header::field_map = {
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