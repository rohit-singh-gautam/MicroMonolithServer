//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2024  Rohit Jairaj Singh (rohit@singh.org.in)          //
//                                                                      //
// This program is free software: you can redistribute it and/or modify //
// it under the terms of the GNU General Public License as published by //
// the Free Software Foundation, either version 3 of the License, or    //
// (at your option) any later version.                                  //
//                                                                      //
// This program is distributed in the hope that it will be useful,      //
// but WITHOUT ANY WARRANTY; without even the implied warranty of       //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        //
// GNU General Public License for more details.                         //
//                                                                      //
// You should have received a copy of the GNU General Public License    //
// along with this program.  If not, see <https://www.gnu.org/licenses/>//
//////////////////////////////////////////////////////////////////////////

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