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
//                                                                                         //
// HPack is based on https://www.rfc-editor.org/rfc/rfc9000.html                           //
//                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/base/stream.h>

namespace MMS::net::quic {

// 16. https://www.rfc-editor.org/rfc/rfc9000.html#name-variable-length-integer-enc
constexpr uint64_t DecodeUnsignedInteger(const Stream &stream) {
    const auto integer_type = *stream >> 6;
    //const auto length = 1 << integer_type;
    switch(integer_type) {
    case 0x00:
        return static_cast<uint64_t>(*stream++) & 0x3fULL;
    case 0x01:
        return ((static_cast<uint64_t>(*stream++) & 0x3fULL) << 8) + static_cast<uint64_t>(*stream++);
    case 0x02:
        return ((static_cast<uint64_t>(*stream++) & 0x3fULL) << 24) + (static_cast<uint64_t>(*stream++) << 16) + (static_cast<uint64_t>(*stream++) << 8) + static_cast<uint64_t>(*stream++);
    
    // case 0x03:
    default:
        return  ((static_cast<uint64_t>(*stream++) & 0x3fULL) << 56) + (static_cast<uint64_t>(*stream++) << 48) + (static_cast<uint64_t>(*stream++) << 40) + (static_cast<uint64_t>(*stream++) << 32) +
                (static_cast<uint64_t>(*stream++) << 24) + (static_cast<uint64_t>(*stream++) << 16) + (static_cast<uint64_t>(*stream++) << 8) + static_cast<uint64_t>(*stream++);
    }
}

template <size_t length>
constexpr inline void EncodeUnsignedInteger(uint64_t value, Stream &stream) {
    if constexpr (length == 1) {
        // Code of 1 byte is 0x00 hence just setting value is sufficient
        *stream++ = value & 0x3f;
    } else if constexpr (length == 2) {
        *stream++ = ((value >> 8) & 0x3f) | 0x40;
        *stream++ = value & 0xff;
    } else if constexpr (length == 4) {
        *stream++ = ((value >> 24) & 0x3f) | 0x80;
        *stream++ = (value >> 16) & 0xff;
        *stream++ = (value >> 8) & 0xff;
        *stream++ = value & 0xff;
    } else if constexpr (length == 8) {
        *stream++ = ((value >> 56) & 0x3f) | 0xc0;
        *stream++ = (value >> 48) & 0xff;
        *stream++ = (value >> 40) & 0xff;
        *stream++ = (value >> 32) & 0xff;
        *stream++ = (value >> 24) & 0xff;
        *stream++ = (value >> 16) & 0xff;
        *stream++ = (value >> 8) & 0xff;
        *stream++ = value & 0xff;
    } else {
        throw QUICEncodeIntgegerFailed { };
    }
}

constexpr void EncodeUnsignedInteger(uint64_t value, Stream &stream) {
    if (value <= 63ULL) {
        // Code of 1 byte is 0x00 hence just setting value is sufficient
        *stream++ = value;
    } else if (value <= 16383ULL) {
        EncodeUnsignedInteger<2>(value, stream);
    } else if (value <= 1073741823) {
        EncodeUnsignedInteger<4>(value, stream);
    } else if (value <= 4611686018427387903) {
        EncodeUnsignedInteger<8>(value, stream);
    } else {
        throw QUICEncodeIntgegerFailed { };
    }
}

} // MMS::net::quic