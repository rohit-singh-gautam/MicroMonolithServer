/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/base/error.h>
#include <mms/base/stream.h>
#include <string_view>

namespace MMS {

std::string http_parser_failed_t::to_string() {
    auto errstr = error_helper_t::to_string() + ": ";

    if (stream.index() >= 40 ){
        const std::string_view initial {reinterpret_cast<const char *>(stream.begin()), 16};
        for(auto &current_ch: initial) {
            if (current_ch >= 32 /* &&  current_ch <= 127 */) {
                errstr.push_back(current_ch);
            } else errstr.push_back('#');
        }
        errstr += " ... ";

        const std::string_view second {reinterpret_cast<const char *>(stream.curr() - 16), 16};
        for(auto &current_ch: initial) {
            if (current_ch >= 32 /* &&  current_ch <= 127 */) {
                errstr.push_back(current_ch);
            } else errstr.push_back('#');
        }
    } else {
        const std::string_view initial {reinterpret_cast<const char *>(stream.begin()), stream.index()};
        for(auto &current_ch: initial) {
            if (current_ch >= 32 /* &&  current_ch <= 127 */) {
                errstr.push_back(current_ch);
            } else errstr.push_back('#');
        }
    }

    errstr += " <-- failed here --|";

    const std::string_view last {reinterpret_cast<const char *>(stream.curr()), std::min<size_t>(16, stream.remaining_buffer())};
    for(auto &current_ch: last) {
        if (current_ch >= 32 /* &&  current_ch <= 127 */) {
            errstr.push_back(current_ch);
        } else errstr.push_back('#');
    }
    if (stream.remaining_buffer() > 16) {
        errstr += " ... more ";
        errstr += std::to_string(stream.remaining_buffer() - 16UL);
        errstr += " characters.";
    }

    return errstr;
}


} // namespace MMS