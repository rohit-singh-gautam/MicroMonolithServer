/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/base/error.h>

namespace MMS {

std::string http_parser_failed_t::to_string(const char *start_position) {
    auto errstr = error_helper_t::to_string() + ": ";

    if (current_position < start_position) {
        errstr += "Underflow - ";
    } else 
    {
        if (current_position - start_position >= 40 ){
            for(size_t index { 0 }; index < 16; ++index) {
                auto current_ch = start_position[index];
                if (current_ch >= 32 /* &&  current_ch <= 127 */) {
                    errstr.push_back(current_ch);
                } else errstr.push_back('#');
            }
            errstr += " ... ";

            for(size_t index { 16 }; index; --index) {
                auto current_ch = *(current_position - index);
                if (current_ch >= 32 /* &&  current_ch <= 127 */) {
                    errstr.push_back(current_ch);
                } else errstr.push_back('#');
            }
        } else {
            auto itr = start_position;
            while(itr < current_position) {
                auto current_ch = *itr;
                if (current_ch >= 32 /* &&  current_ch <= 127 */) {
                    errstr.push_back(current_ch);
                } else errstr.push_back(current_ch);

                ++itr;
            }
        }

        errstr += " <-- error is here -- ";
    }

    for(size_t index { 0 }; index < std::min(16UL, buffer_remaining); ++index) {
        auto current_ch = current_position[index];
        if (current_ch >= 32 /* &&  current_ch <= 127 */) {
            errstr.push_back(current_ch);
        } else errstr.push_back('#');
    }
    if (buffer_remaining > 16) {
        errstr += " ... more " + std::to_string(buffer_remaining - 16UL) + " characters.";
    }

    return errstr;
}


} // namespace MMS