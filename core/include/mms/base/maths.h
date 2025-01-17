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

#pragma once
#include <mms/base/types.h>
#include <stdint.h>
#include <type_traits>
#include <cmath>

namespace MMS {

enum class number_case_t {
    lower,
    upper
};

namespace constant {

constexpr char lower_case_numbers[] = {
    '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','g','h',
    'i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
constexpr char upper_case_numbers[] = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H',
    'I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

constexpr uint8_t char_to_int[] {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 00  - 15
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 16  - 31
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 32  - 47
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 48  - 63
    0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 64  - 79
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 80  - 95
    0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 96  - 111
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 112 - 127
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 128 - 143
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 144 - 159
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 160 - 175
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 176 - 191
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 192 - 207
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 208 - 223
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 224 - 239
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 240 - 255
};

constexpr char to_base64[] {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

constexpr uint8_t from_base64[] {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 00  - 15
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 16  - 31
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x3f, // 32  - 47
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 48  - 63
    0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, // 64  - 79
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, // 80  - 95
    0x00, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, // 96  - 111
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, // 112 - 127
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 128 - 143
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 144 - 159
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 160 - 175
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 176 - 191
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 192 - 207
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 208 - 223
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 224 - 239
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 240 - 255
};

} // namespace constant

template <std::integral T, T radix = 10, number_case_t number_case = number_case_t::lower, bool null_terminated = true>
constexpr size_t to_string(T val, typecheck::byte auto * const dest) {
    static_assert(!std::is_signed<T>::value || (std::is_signed<T>::value && radix == 10), "Signed type only allowed for radix 10" );
    static_assert(radix >= 2, "Radix must be atleast 2");
    static_assert(radix <= 36, "Radix more than 36 not supported");
    auto dest_ptr = dest;

    decltype(val) val1;
    if constexpr (std::is_signed<T>::value) val1 = abs(val);
    else val1 = val;

    do {
        decltype(val) modval = val1 % radix;
        if constexpr (number_case == number_case_t::lower)
            *dest_ptr++ = constant::lower_case_numbers[modval];
        else
            *dest_ptr++ = constant::upper_case_numbers[modval];
    } while((val1 /= radix));

    if constexpr (std::is_signed<T>::value) {
        if (val < 0) *dest_ptr++ = '-';
    }
    std::reverse(dest , dest_ptr);

    if constexpr (null_terminated == true) {
        *dest_ptr++ = '\0';
    }
    return (size_t)(dest_ptr - dest);
}

template <bool null_terminated = true> 
constexpr size_t to_string(std::floating_point auto val, typecheck::byte auto *dest) {
    size_t ret = sprintf(dest, "%f", val);

    if constexpr (null_terminated == true) {
        ++ret;
    }

    return ret;
}

template <std::unsigned_integral T, T radix = 10>
constexpr T to_uint(const char *src, size_t *len = nullptr) {
    T val {  };
    uint8_t current = 0;
    auto psrc = reinterpret_cast<const uint8_t *>(src);
    if ((current = *psrc++)) {
        val = constant::char_to_int[current];
        while((current = *psrc++)) {
            val = val * radix + constant::char_to_int[current];
        }
    }

    if (len != nullptr) *len = (size_t)(psrc - reinterpret_cast<const uint8_t *>(src));

    return val;
}

template <std::signed_integral T, T radix = 10>
constexpr T to_int(const char *src, size_t *len = nullptr) {
    T val = 0;
    char current = 0;
    const char *psrc = src;

    //Checking for sign
    T sign = 1;
    switch(*psrc) {
    case '-':
        sign = -1;
        ++psrc;
        break;
    case '+':
        ++psrc;
        break;
    default:
        break;
    }

    if ((current = *psrc++)) {
        val = constant::char_to_int[(size_t)current];
        while((current = *psrc++)) {
            val = val * radix + constant::char_to_int[(size_t)current];
        }
    }

    if (len != nullptr) *len = (size_t)(psrc - src);

    return val * sign;
}

template <std::integral T, bool null_terminated = true>
constexpr size_t to_string64_hash() {
    size_t value = (sizeof(T) * 8 + 5) / 6;
    if constexpr (null_terminated) {
        ++value;
    }
    return value;
}

template <std::integral T, bool null_terminated = true>
constexpr size_t to_string64_hash(T val, char * const dest) {
    char * dest_ptr = dest;
    constexpr auto total_bits = sizeof(T) * 8;

    *dest_ptr++ = constant::to_base64[val & 0x3f];

    size_t current_bit = 6;
    do {
        *dest_ptr++ = constant::to_base64[(val >> current_bit) & 0x3f];
        current_bit += 6;
    } while(current_bit < total_bits);

    if constexpr (null_terminated == true) {
        *dest_ptr++ = '\0';
    }
    
    return (size_t)(dest_ptr - dest);
}

constexpr size_t base64_decode_len(const typecheck::byte auto * const buffer, const size_t buffer_len) {
    if (buffer_len < 4) return 0;
    size_t decode_len = (buffer_len * 3) / 4;
    if (buffer[buffer_len - 2] == '=') return decode_len - 2;
    if (buffer[buffer_len - 1] == '=') return decode_len - 1;

    return decode_len;
}

constexpr uint8_t * base64_decode(const typecheck::byte auto *const buffer, const size_t buffer_len, uint8_t *dest) {
    auto pstart = buffer;
    // Just making sure that len is multiple of 4
    // Bad request can cause buffer overflow
    auto pend = buffer + (buffer_len & (~0x03));

    while(pstart < pend) {
        auto val1 = constant::from_base64[*pstart++];
        auto val2 = constant::from_base64[*pstart++];
        *dest++ = (val1 << 2) + (val2 >> 4);
        if (*pstart == '=') {
            ++pstart;
            ++pstart;
            break;
        }

        auto val3 = constant::from_base64[*pstart++];
        *dest++ = (val2 << 4) + (val3 >> 2);
        if (*pstart == '=') {
            ++pstart;
            break;
        }

        auto val4 = constant::from_base64[*pstart++];
        *dest++ = (val3 << 6) + val4;
    }

    return dest;
}

} // namespace MMS