/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <iostream>
#include <string>

#include <rohit/base/error.h>
#include <http11parser.h>
#include <http11scanner.h>
#include <http11.h>

namespace rohit {

class http11driver {
public:
    ~http11driver();

    err_t parse(std::string &text);

    http_header_request header { };

private:
    err_t parse_internal(std::istream &iss);

    rohit::parser *parser  = nullptr;
    rohit::http11scanner *scanner = nullptr;

public:
    friend std::ostream& operator<<(std::ostream& os, const http11driver& driver);
};

inline std::ostream& operator<<(std::ostream& os, const http11driver& driver) { return os << driver.header; }

inline const char *skipFirstAndSpace(const char *str) {
    ++str;
    while(*str && (*str == ' ' || *str == '\t')) ++str;

    return str;
}

} // namespace rohit