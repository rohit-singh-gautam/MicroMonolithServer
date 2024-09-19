/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                         //
// HPack is based on https://www.rfc-editor.org/rfc/rfc7541.html                           //
//                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <http/httpparser.h>
#include <string>
#include <vector>
#include <mms/base/error.h>
#include <mms/listener.h>

namespace std {
template<>
struct hash<std::pair<MMS::http::FIELD, std::string>>
{
    auto
    operator()(const std::pair<MMS::http::FIELD, std::string> &val) const noexcept
    {
        return std::_Hash_impl::__hash_combine(val.first, std::hash<std::string>{}(val.second));
    }
};
} // namespace std

namespace MMS::http::hpack {

class map_table_t {
private:
    std::vector<std::pair<FIELD, std::string>> entries { };
    std::unordered_map<std::pair<FIELD, std::string>, size_t> entry_value_map { };

    // Field string -> index, count
    // Count will be used for dynamic to cleanup
    std::unordered_map<FIELD, std::pair<size_t, size_t>> entry_map { };

    static const std::pair<FIELD, std::string> empty;

public:
    inline map_table_t() { }
    inline map_table_t(const std::initializer_list<std::pair<FIELD, std::string>> &list) : entries { list } { }
    map_table_t(const map_table_t &) = delete;
    map_table_t &operator=(const map_table_t &) = delete;

    inline const auto &operator[](const size_t index) const {
        if (index >= entries.size()) {
            // TODO: Add exceptions instead of returning empty
            return empty;
        } else {
            return entries[index];
        }
    }

    inline size_t operator[](const std::pair<FIELD, std::string> &header_line) const {
        auto entry_itr = entry_value_map.find(header_line);
        if (entry_itr == entry_value_map.end()) {
            // TODO: Add exceptions instead of returning -1
            return -1;
        }
        return entry_itr->second;
    }

    inline size_t operator[](const FIELD &field) const {
        auto entry_itr = entry_map.find(field);
        if (entry_itr == entry_map.end()) {
            // TODO: Add exceptions instead of returning -1
            return -1;
        }
        return entry_itr->second.first;
    }

    inline void push_back(const std::pair<FIELD, std::string> &entry) {
        entries.push_back(entry);
        const size_t index = entries.size() - 1;
        if (entry.second != "") {
            entry_value_map.insert(std::make_pair(entry, index));
        }
        auto entry_itr = entry_map.find(entry.first);
        if (entry_itr == entry_map.end()) {
            entry_map.insert(std::make_pair(entry.first, std::make_pair(index, 1)));
        } else {
            entry_itr->second.second++;
        }
    }

    inline void pop_back() {
        const auto entry = entries.back();
        entries.pop_back();
        
        if (entry.second != "") {
            entry_value_map.erase(entry);
        }

#if defined(DEBUG) || defined(NDEBUG)
        auto entry_itr = entry_map.find(entry.first);
        if (entry_itr != entry_map.end()) {
            if (entry_itr->second.second == 1) {
                entry_map.erase(entry.first);
            } else {
                entry_itr->second.second--;
            }
        } else {
            throw exception_t(err_t::HTTP2_HPACK_TABLE_ERROR);
        }
#else
        auto entry_itr = entry_map.find(entry.first);
        if (entry_itr->second.second == 1) {
            entry_map.erase(entry.first);
        } else {
            entry_itr->second.second--;
        }
#endif
    }

    inline bool contains(const std::pair<FIELD, std::string> &entry) const {
        return entry_value_map.contains(entry);
    }

    inline bool contains(const FIELD &entry) const {
        return entry_map.contains(entry);
    }

    inline void clear() {
        entries.clear();
        entry_value_map.clear();
        entry_map.clear();
    }

    inline auto size() const {
        return entries.size();
    }

};

class dynamic_table_t : public map_table_t {
private:
    size_t max_size;

public:
    dynamic_table_t(size_t max_size = 12) : map_table_t { }, max_size { max_size } {}

    inline void update_size(size_t new_size) {
        if (max_size == new_size) return;
        max_size = new_size;
        while (new_size < size()) {
            pop_back();
        }
    }

    inline void insert(const std::pair<FIELD, std::string> &entry) {
        if (size() >= max_size) {
            // rfc7541 - 4.4.  Entry Eviction When Adding New Entries
            clear();
        }
        push_back(entry);
    }
};

class static_table_t : public map_table_t {
public:
    using map_table_t::map_table_t;
};

extern const static_table_t static_table;

// Static table entry defined in
// https://www.rfc-editor.org/rfc/rfc7541.html#appendix-A
#define HTTP2_STATIC_TABLE_LIST \
    HTTP2_STATIC_TABLE_ENTRY( 0, FIELD::IGNORE_THIS, "") /* 0 is not used */ \
    HTTP2_STATIC_TABLE_ENTRY( 1, FIELD::Authority, "") \
    HTTP2_STATIC_TABLE_ENTRY( 2, FIELD::Method, "GET") \
    HTTP2_STATIC_TABLE_ENTRY( 3, FIELD::Method, "POST") \
    HTTP2_STATIC_TABLE_ENTRY( 4, FIELD::Path, "/") \
    HTTP2_STATIC_TABLE_ENTRY( 5, FIELD::Path, "/index.html") \
    HTTP2_STATIC_TABLE_ENTRY( 6, FIELD::Scheme,  "http") \
    HTTP2_STATIC_TABLE_ENTRY( 7, FIELD::Scheme, "https") \
    HTTP2_STATIC_TABLE_ENTRY( 8, FIELD::Status, "200") \
    HTTP2_STATIC_TABLE_ENTRY( 9, FIELD::Status, "204") \
    HTTP2_STATIC_TABLE_ENTRY(10, FIELD::Status, "206") /* 10 */ \
    HTTP2_STATIC_TABLE_ENTRY(11, FIELD::Status, "304") \
    HTTP2_STATIC_TABLE_ENTRY(12, FIELD::Status, "400") \
    HTTP2_STATIC_TABLE_ENTRY(13, FIELD::Status, "404") \
    HTTP2_STATIC_TABLE_ENTRY(14, FIELD::Status, "500") \
    HTTP2_STATIC_TABLE_ENTRY(15, FIELD::Accept_Charset, "") \
    HTTP2_STATIC_TABLE_ENTRY(16, FIELD::Accept_Encoding, "gzip, deflate") \
    HTTP2_STATIC_TABLE_ENTRY(17, FIELD::Accept_Language, "") \
    HTTP2_STATIC_TABLE_ENTRY(18, FIELD::Accept_Ranges, "") \
    HTTP2_STATIC_TABLE_ENTRY(19, FIELD::Accept, "") \
    HTTP2_STATIC_TABLE_ENTRY(20, FIELD::Access_Control_Allow_Origin, "") /* 20 */ \
    HTTP2_STATIC_TABLE_ENTRY(21, FIELD::Age, "") \
    HTTP2_STATIC_TABLE_ENTRY(22, FIELD::Allow, "") \
    HTTP2_STATIC_TABLE_ENTRY(23, FIELD::Authorization, "") \
    HTTP2_STATIC_TABLE_ENTRY(24, FIELD::Cache_Control, "") \
    HTTP2_STATIC_TABLE_ENTRY(25, FIELD::Content_Disposition, "") \
    HTTP2_STATIC_TABLE_ENTRY(26, FIELD::Content_Encoding, "") \
    HTTP2_STATIC_TABLE_ENTRY(27, FIELD::Content_Language, "") \
    HTTP2_STATIC_TABLE_ENTRY(28, FIELD::Content_Length, "") \
    HTTP2_STATIC_TABLE_ENTRY(29, FIELD::Content_Location, "") \
    HTTP2_STATIC_TABLE_ENTRY(30, FIELD::Content_Range, "") /* 30 */ \
    HTTP2_STATIC_TABLE_ENTRY(31, FIELD::Content_Type, "") \
    HTTP2_STATIC_TABLE_ENTRY(32, FIELD::Cookie, "") \
    HTTP2_STATIC_TABLE_ENTRY(33, FIELD::Date, "") \
    HTTP2_STATIC_TABLE_ENTRY(34, FIELD::ETag, "") \
    HTTP2_STATIC_TABLE_ENTRY(35, FIELD::Expect, "") \
    HTTP2_STATIC_TABLE_ENTRY(36, FIELD::Expires, "") \
    HTTP2_STATIC_TABLE_ENTRY(37, FIELD::From, "") \
    HTTP2_STATIC_TABLE_ENTRY(38, FIELD::Host, "") \
    HTTP2_STATIC_TABLE_ENTRY(39, FIELD::If_Match, "") \
    HTTP2_STATIC_TABLE_ENTRY(40, FIELD::If_Modified_Since, "") /* 40 */ \
    HTTP2_STATIC_TABLE_ENTRY(41, FIELD::If_None_Match, "") \
    HTTP2_STATIC_TABLE_ENTRY(42, FIELD::If_Range, "") \
    HTTP2_STATIC_TABLE_ENTRY(43, FIELD::If_Unmodified_Since, "") \
    HTTP2_STATIC_TABLE_ENTRY(44, FIELD::Last_Modified, "") \
    HTTP2_STATIC_TABLE_ENTRY(45, FIELD::Link, "") \
    HTTP2_STATIC_TABLE_ENTRY(46, FIELD::Location, "") \
    HTTP2_STATIC_TABLE_ENTRY(47, FIELD::Max_Forwards, "") \
    HTTP2_STATIC_TABLE_ENTRY(48, FIELD::Proxy_Authenticate, "") \
    HTTP2_STATIC_TABLE_ENTRY(49, FIELD::Proxy_Authorization, "") \
    HTTP2_STATIC_TABLE_ENTRY(50, FIELD::Range, "") /* 50 */ \
    HTTP2_STATIC_TABLE_ENTRY(51, FIELD::Referer, "") \
    HTTP2_STATIC_TABLE_ENTRY(52, FIELD::Refresh, "") \
    HTTP2_STATIC_TABLE_ENTRY(53, FIELD::Retry_After, "") \
    HTTP2_STATIC_TABLE_ENTRY(54, FIELD::Server, "") \
    HTTP2_STATIC_TABLE_ENTRY(55, FIELD::Set_Cookie, "") \
    HTTP2_STATIC_TABLE_ENTRY(56, FIELD::Strict_Transport_Security, "") \
    HTTP2_STATIC_TABLE_ENTRY(57, FIELD::Transfer_Encoding, "") \
    HTTP2_STATIC_TABLE_ENTRY(58, FIELD::User_Agent, "") \
    HTTP2_STATIC_TABLE_ENTRY(59, FIELD::Vary, "") \
    HTTP2_STATIC_TABLE_ENTRY(60, FIELD::Via, "") /* 60 */ \
    HTTP2_STATIC_TABLE_ENTRY(61, FIELD::WWW_Authenticate, "") \
    LIST_DEFINITION_END

class node {
    int16_t symbol { -1 };

public:
    node *left { nullptr };
    node *right { nullptr };

    constexpr node() { }

    constexpr auto is_leaf() const { return left == nullptr && right == nullptr; }
    
    constexpr void set_leaf(const uint16_t symbol) {
        this->symbol = symbol;
    }

    constexpr auto is_eos() const { return symbol == 256; }
    constexpr auto get_symbol() const { return static_cast<char>(symbol); }
};

// Integer representation, decode and encode is defined in
// https://www.rfc-editor.org/rfc/rfc7541.html#section-5.1
template <uint32_t N>
constexpr auto decode_integer(const Stream &stream) {
    constexpr uint32_t mask { (1 << N) - 1 };
    uint32_t value { *stream++ & mask };
    if (value < mask) {
        return value;
    }
    value = mask;
    while(!stream.full()) {
        if ((*stream & 0x80) != 0) {
            value = *stream++ & 0x7f;
        } else {
            value = *stream++;
            break;
        }
    }
    return value;
}

template <uint32_t N>
constexpr void encode_integer(Stream &stream, const uint8_t head, std::unsigned_integral auto value) {
    constexpr uint32_t mask { (1 << N) - 1 };
    if (value < mask) {
        *stream++ = head + static_cast<uint8_t>(value);
    } else {
        *stream++ = head + mask;
        value -= mask;
        while(value >= 128) {
            *stream++ = 0x80 & (value % 128);
            value >>= 7;
        }
        *stream++ = value;
    }
}

// String representation is defined in
// https://www.rfc-editor.org/rfc/rfc7541.html#section-5.2
struct huffman_entry {
    const uint32_t code;
    const uint32_t code_len;

    constexpr huffman_entry(const uint32_t code, const uint32_t code_len) : code(code), code_len(code_len) { }
};

// Huffnam table entry defined at
// https://www.rfc-editor.org/rfc/rfc7541.html#appendix-B
constexpr const huffman_entry static_huffman[] = {
    {    0x1ff8, 13}, {  0x7fffd8, 23}, { 0xfffffe2, 28}, { 0xfffffe3, 28}, { 0xfffffe4, 28}, { 0xfffffe5, 28}, { 0xfffffe5, 28}, { 0xfffffe7, 28}, //000-007
    { 0xfffffe8, 28}, {  0xffffea, 24}, {0x3ffffffc, 30}, { 0xfffffe9, 28}, { 0xfffffea, 28}, {0x3ffffffd, 30}, { 0xfffffeb, 28}, { 0xfffffec, 28}, //008-015
    { 0xfffffed, 28}, { 0xfffffee, 28}, { 0xfffffef, 28}, { 0xffffff0, 28}, { 0xffffff1, 28}, { 0xffffff2, 28}, {0x3ffffffe, 30}, { 0xffffff3, 28}, //016-023
    { 0xffffff4, 28}, { 0xffffff5, 28}, { 0xffffff6, 28}, { 0xffffff7, 28}, { 0xffffff8, 28}, { 0xffffff9, 28}, { 0xffffffa, 28}, { 0xffffffb, 28}, //024-031
    {      0x14,  6}, {     0x3f8, 10}, {     0x3f9, 10}, {     0xffa, 12}, {    0x1ff9, 13}, {      0x15,  6}, {      0xf8,  8}, {     0x7fa, 11}, //032-039
    {     0x3fa, 10}, {     0x3fb, 10}, {      0xf9,  8}, {     0x7fb, 11}, {      0xfa,  8}, {      0x16,  6}, {      0x17,  6}, {      0x18,  6}, //040-047
    {       0x0,  5}, {       0x1,  5}, {       0x2,  5}, {      0x19,  6}, {      0x1a,  6}, {      0x1b,  6}, {      0x1c,  6}, {      0x1d,  6}, //048-055
    {      0x1e,  6}, {      0x1f,  6}, {      0x5c,  7}, {      0xfb,  8}, {    0x7ffc, 15}, {      0x20,  6}, {     0xffb, 12}, {     0x3fc, 10}, //056-063
    {    0x1ffa, 13}, {      0x21,  6}, {      0x5d,  7}, {      0x5e,  7}, {      0x5f,  7}, {      0x60,  7}, {      0x61,  7}, {      0x62,  7}, //064-071
    {      0x63,  7}, {      0x64,  7}, {      0x65,  7}, {      0x66,  7}, {      0x67,  7}, {      0x68,  7}, {      0x69,  7}, {      0x6a,  7}, //072-079
    {      0x6b,  7}, {      0x6c,  7}, {      0x6d,  7}, {      0x6e,  7}, {      0x6f,  7}, {      0x70,  7}, {      0x71,  7}, {      0x72,  7}, //080-087
    {      0xfc,  8}, {      0x73,  7}, {      0xfd,  8}, {    0x1ffb, 13}, {   0x7fff0, 19}, {    0x1ffc, 13}, {    0x3ffc, 14}, {      0x22,  6}, //088-095
    {    0x7ffd, 15}, {       0x3,  5}, {      0x23,  6}, {       0x4,  5}, {      0x24,  6}, {       0x5,  5}, {      0x25,  6}, {      0x26,  6}, //096-103
    {      0x27,  6}, {       0x6,  5}, {      0x74,  7}, {      0x75,  7}, {      0x28,  6}, {      0x29,  6}, {      0x2a,  6}, {       0x7,  5}, //104-111
    {      0x2b,  6}, {      0x76,  7}, {      0x2c,  6}, {       0x8,  5}, {       0x9,  5}, {      0x2d,  6}, {      0x77,  7}, {      0x78,  7}, //112-119
    {      0x79,  7}, {      0x7a,  7}, {      0x7b,  7}, {    0x7ffe, 15}, {     0x7fc, 11}, {    0x3ffd, 14}, {    0x1ffd, 13}, { 0xffffffc, 28}, //120-127
    {   0xfffe6, 20}, {  0x3fffd2, 22}, {   0xfffe7, 20}, {   0xfffe8, 20}, {  0x3fffd3, 22}, {  0x3fffd4, 22}, {  0x3fffd5, 22}, {  0x7fffd9, 23}, //128-135
    {  0x3fffd6, 22}, {  0x7fffda, 23}, {  0x7fffdb, 23}, {  0x7fffdc, 23}, {  0x7fffdd, 23}, {  0x7fffde, 23}, {  0xffffeb, 24}, {  0x7fffdf, 23}, //136-143
    {  0xffffec, 24}, {  0xffffed, 24}, {  0x3fffd7, 22}, {  0x7fffe0, 23}, {  0xffffee, 24}, {  0x7fffe1, 23}, {  0x7fffe2, 23}, {  0x7fffe3, 23}, //144-151
    {  0x7fffe4, 23}, {  0x1fffdc, 21}, {  0x3fffd8, 22}, {  0x7fffe5, 23}, {  0x3fffd9, 22}, {  0x7fffe6, 23}, {  0x7fffe7, 23}, {  0xffffef, 24}, //152-159
    {  0x3fffda, 22}, {  0x1fffdd, 21}, {   0xfffe9, 20}, {  0x3fffdb, 22}, {  0x3fffdc, 22}, {  0x7fffe8, 23}, {  0x7fffe9, 23}, {  0x1fffde, 21}, //160-167
    {  0x7fffea, 23}, {  0x3fffdd, 22}, {  0x3fffde, 22}, {  0xfffff0, 24}, {  0x1fffdf, 21}, {  0x3fffdf, 22}, {  0x7fffeb, 23}, {  0x7fffec, 23}, //168-175
    {  0x1fffe0, 21}, {  0x1fffe1, 21}, {  0x3fffe0, 22}, {  0x1fffe2, 21}, {  0x7fffed, 23}, {  0x3fffe1, 22}, {  0x7fffee, 23}, {  0x7fffef, 23}, //176-183
    {   0xfffea, 20}, {  0x3fffe2, 22}, {  0x3fffe3, 22}, {  0x3fffe4, 22}, {  0x7ffff0, 23}, {  0x3fffe5, 22}, {  0x3fffe6, 22}, {  0x7ffff1, 23}, //184-191
    { 0x3ffffe0, 26}, { 0x3ffffe1, 26}, {   0xfffeb, 20}, {   0x7fff1, 19}, {  0x3fffe7, 22}, {  0x7ffff2, 23}, {  0x3fffe8, 22}, { 0x1ffffec, 25}, //192-199
    { 0x3ffffe2, 26}, { 0x3ffffe3, 26}, { 0x3ffffe4, 26}, { 0x7ffffde, 27}, { 0x7ffffdf, 27}, { 0x3ffffe5, 26}, {  0xfffff1, 24}, { 0x1ffffed, 25}, //200-207
    {   0x7fff2, 19}, {  0x1fffe3, 21}, { 0x3ffffe6, 26}, { 0x7ffffe0, 27}, { 0x7ffffe1, 27}, { 0x3ffffe7, 26}, { 0x7ffffe2, 27}, {  0xfffff2, 24}, //208-215
    {  0x1fffe4, 21}, {  0x1fffe5, 21}, { 0x3ffffe8, 26}, { 0x3ffffe9, 26}, { 0xffffffd, 28}, { 0x7ffffe3, 27}, { 0x7ffffe4, 27}, { 0x7ffffe5, 27}, //216-223
    {   0xfffec, 20}, {  0xfffff3, 24}, {   0xfffed, 20}, {  0x1fffe6, 21}, {  0x3fffe9, 22}, {  0x1fffe7, 21}, {  0x1fffe8, 21}, {  0x7ffff3, 23}, //224-231
    {  0x3fffea, 22}, {  0x3fffeb, 22}, { 0x1ffffee, 25}, { 0x1ffffef, 25}, {  0xfffff4, 24}, {  0xfffff5, 24}, { 0x3ffffea, 26}, {  0x7ffff4, 23}, //232-239
    { 0x3ffffeb, 26}, { 0x7ffffe6, 27}, { 0x3ffffec, 26}, { 0x3ffffed, 26}, { 0x7ffffe7, 27}, { 0x7ffffe8, 27}, { 0x7ffffe9, 27}, { 0x7ffffea, 27}, //240-247
    { 0x7ffffeb, 27}, { 0xffffffe, 28}, { 0x7ffffec, 27}, { 0x7ffffed, 27}, { 0x7ffffee, 27}, { 0x7ffffef, 27}, { 0x7fffff0, 27}, { 0x3ffffee, 26}, //248-255
    {0x3fffffff, 30}                                                                                                                                //256
};

node *created_huffman_tree();

extern const node *huffman_root;

std::string get_huffman_string(const Stream &stream);

//! N must include H
// +---+---------------------------+
// | H |     Value Length (7+)     |
// +---+---------------------------+
// N must be 8 for above example
//  +---+---+---+---+---+---+---+---+
// | 0 | 0 | 1 | N | H |NameLen(3+)|
// +---+---+---+---+---+-----------+
// N must be 4 for above example
template <uint32_t N>
inline auto get_header_string(const Stream &stream) {
    const size_t len = hpack::decode_integer<N - 1>(stream);
    bool H = *stream & (1 << (N - 1));
    ++stream;
    std::string value = H ? get_huffman_string(stream.GetSimpleStream()) : std::string { reinterpret_cast<const char *>(stream.curr()), len };
    stream += len;
    return value;
}

inline size_t huffman_string_size(const Stream &stream) {
    size_t size { 7 };
    while(!stream.full()) size += static_huffman[*stream++].code_len;
    return size / 8;
}

void add_huffman_string(Stream &stream, const Stream &valstream);

inline auto add_header_string(Stream &stream, const std::string &value) {
    auto strstream = make_const_fullstream(value);
    size_t size = huffman_string_size(strstream);
    if (size < value.size()) {
        // Encoded string is smaller hence we are encoded
        encode_integer<7>(stream, static_cast<uint8_t>(0x80), size);
        strstream.Reset();
        add_huffman_string(stream, strstream);
    } else {
        encode_integer<7>(stream, static_cast<uint8_t>(0x80), value.size());
        stream.curr() = std::copy(value.begin(), value.end(), stream.curr());
    }
}

inline auto get_header_field(const Stream &stream) {
    auto header_string = get_header_string<8>(stream);
    return to_field(header_string);
}

} // namespace rohit::http::hpack