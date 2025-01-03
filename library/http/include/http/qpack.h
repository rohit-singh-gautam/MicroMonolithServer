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
//
// This header implements
// https://www.rfc-editor.org/rfc/rfc9204.html
//
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <http/httpparser.h>
#include <mms/base/error.h>
#include <mms/listener.h>
#include <http/hpack.h>
#include <http/httpdef.h>

namespace MMS::http::qpack {

using MMS::http::FIELD;
using MMS::http::METHOD;
using MMS::http::CODE;

using MMS::http::hpack::dynamic_table_t;
using MMS::http::hpack::static_table_t;

#ifndef LIST_DEFINITION_END
#define LIST_DEFINITION_END
#endif

extern const static_table_t static_table;

// https://www.rfc-editor.org/rfc/rfc9204.html#static-table
#define QPACK_STATIC_TABLE_LIST \
    QPACK_STATIC_TABLE_ENTRY( 0, FIELD::Authority, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 1, FIELD::Path, "/" ) \
    QPACK_STATIC_TABLE_ENTRY( 2, FIELD::Age, "0" ) \
    QPACK_STATIC_TABLE_ENTRY( 3, FIELD::Content_Disposition, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 4, FIELD::Content_Length, "0" ) \
    QPACK_STATIC_TABLE_ENTRY( 5, FIELD::Cookie, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 6, FIELD::Date, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 7, FIELD::ETag, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 8, FIELD::If_Modified_Since, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 9, FIELD::If_None_Match, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 10, FIELD::Last_Modified, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 11, FIELD::Link, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 12, FIELD::Location, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 13, FIELD::Referer, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 14, FIELD::Set_Cookie, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 15, FIELD::Method, "CONNECT" ) \
    QPACK_STATIC_TABLE_ENTRY( 16, FIELD::Method, "DELETE" ) \
    QPACK_STATIC_TABLE_ENTRY( 17, FIELD::Method, "GET" ) \
    QPACK_STATIC_TABLE_ENTRY( 18, FIELD::Method, "HEAD" ) \
    QPACK_STATIC_TABLE_ENTRY( 19, FIELD::Method, "OPTIONS" ) \
    QPACK_STATIC_TABLE_ENTRY( 20, FIELD::Method, "POST" ) \
    QPACK_STATIC_TABLE_ENTRY( 21, FIELD::Method, "PUT" ) \
    QPACK_STATIC_TABLE_ENTRY( 22, FIELD::Scheme, "http" ) \
    QPACK_STATIC_TABLE_ENTRY( 23, FIELD::Scheme, "https" ) \
    QPACK_STATIC_TABLE_ENTRY( 24, FIELD::Status, "103" ) \
    QPACK_STATIC_TABLE_ENTRY( 25, FIELD::Status, "200" ) \
    QPACK_STATIC_TABLE_ENTRY( 26, FIELD::Status, "304" ) \
    QPACK_STATIC_TABLE_ENTRY( 27, FIELD::Status, "404" ) \
    QPACK_STATIC_TABLE_ENTRY( 28, FIELD::Status, "503" ) \
    QPACK_STATIC_TABLE_ENTRY( 29, FIELD::Accept, "*/*" ) \
    QPACK_STATIC_TABLE_ENTRY( 30, FIELD::Accept, "application/dns-message" ) \
    QPACK_STATIC_TABLE_ENTRY( 31, FIELD::Accept_Encoding, "gzip, deflate, br" ) \
    QPACK_STATIC_TABLE_ENTRY( 32, FIELD::Accept_Ranges, "bytes" ) \
    QPACK_STATIC_TABLE_ENTRY( 33, FIELD::Access_Control_Allow_Headers, "cache-control" ) \
    QPACK_STATIC_TABLE_ENTRY( 34, FIELD::Access_Control_Allow_Headers, "content-type" ) \
    QPACK_STATIC_TABLE_ENTRY( 35, FIELD::Access_Control_Allow_Origin, "*" ) \
    QPACK_STATIC_TABLE_ENTRY( 36, FIELD::Cache_Control, "max-age=0" ) \
    QPACK_STATIC_TABLE_ENTRY( 37, FIELD::Cache_Control, "max-age=2592000" ) \
    QPACK_STATIC_TABLE_ENTRY( 38, FIELD::Cache_Control, "max-age=604800" ) \
    QPACK_STATIC_TABLE_ENTRY( 39, FIELD::Cache_Control, "no-cache" ) \
    QPACK_STATIC_TABLE_ENTRY( 40, FIELD::Cache_Control, "no-store" ) \
    QPACK_STATIC_TABLE_ENTRY( 41, FIELD::Cache_Control, "public, max-age=31536000" ) \
    QPACK_STATIC_TABLE_ENTRY( 42, FIELD::Content_Encoding, "br" ) \
    QPACK_STATIC_TABLE_ENTRY( 43, FIELD::Content_Encoding, "gzip" ) \
    QPACK_STATIC_TABLE_ENTRY( 44, FIELD::Content_Type, "application/dns-message" ) \
    QPACK_STATIC_TABLE_ENTRY( 45, FIELD::Content_Type, "application/javascript" ) \
    QPACK_STATIC_TABLE_ENTRY( 46, FIELD::Content_Type, "application/json" ) \
    QPACK_STATIC_TABLE_ENTRY( 47, FIELD::Content_Type, "application/x-www-form-urlencoded" ) \
    QPACK_STATIC_TABLE_ENTRY( 48, FIELD::Content_Type, "image/gif" ) \
    QPACK_STATIC_TABLE_ENTRY( 49, FIELD::Content_Type, "image/jpeg" ) \
    QPACK_STATIC_TABLE_ENTRY( 50, FIELD::Content_Type, "image/png" ) \
    QPACK_STATIC_TABLE_ENTRY( 51, FIELD::Content_Type, "text/css" ) \
    QPACK_STATIC_TABLE_ENTRY( 52, FIELD::Content_Type, "text/html; charset=utf-8" ) \
    QPACK_STATIC_TABLE_ENTRY( 53, FIELD::Content_Type, "text/plain" ) \
    QPACK_STATIC_TABLE_ENTRY( 54, FIELD::Content_Type, "text/plain;charset=utf-8" ) \
    QPACK_STATIC_TABLE_ENTRY( 55, FIELD::Range, "bytes=0-" ) \
    QPACK_STATIC_TABLE_ENTRY( 56, FIELD::Strict_Transport_Security, "max-age=31536000" ) \
    QPACK_STATIC_TABLE_ENTRY( 57, FIELD::Strict_Transport_Security, "max-age=31536000; includesubdomains" ) \
    QPACK_STATIC_TABLE_ENTRY( 58, FIELD::Strict_Transport_Security, "max-age=31536000; includesubdomains; preload" ) \
    QPACK_STATIC_TABLE_ENTRY( 59, FIELD::Vary, "accept-encoding" ) \
    QPACK_STATIC_TABLE_ENTRY( 60, FIELD::Vary, "origin" ) \
    QPACK_STATIC_TABLE_ENTRY( 61, FIELD::X_Content_Type_Options, "nosniff" ) \
    QPACK_STATIC_TABLE_ENTRY( 62, FIELD::X_XSS_Protection, "1; mode=block" ) \
    QPACK_STATIC_TABLE_ENTRY( 63, FIELD::Status, "100" ) \
    QPACK_STATIC_TABLE_ENTRY( 64, FIELD::Status, "204" ) \
    QPACK_STATIC_TABLE_ENTRY( 65, FIELD::Status, "206" ) \
    QPACK_STATIC_TABLE_ENTRY( 66, FIELD::Status, "302" ) \
    QPACK_STATIC_TABLE_ENTRY( 67, FIELD::Status, "400" ) \
    QPACK_STATIC_TABLE_ENTRY( 68, FIELD::Status, "403" ) \
    QPACK_STATIC_TABLE_ENTRY( 69, FIELD::Status, "421" ) \
    QPACK_STATIC_TABLE_ENTRY( 70, FIELD::Status, "425" ) \
    QPACK_STATIC_TABLE_ENTRY( 71, FIELD::Status, "500" ) \
    QPACK_STATIC_TABLE_ENTRY( 72, FIELD::Accept_Language, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 73, FIELD::Access_Control_Allow_Credentials, "FALSE" ) \
    QPACK_STATIC_TABLE_ENTRY( 74, FIELD::Access_Control_Allow_Credentials, "TRUE" ) \
    QPACK_STATIC_TABLE_ENTRY( 75, FIELD::Access_Control_Allow_Headers, "*" ) \
    QPACK_STATIC_TABLE_ENTRY( 76, FIELD::Access_Control_Allow_Methods, "get" ) \
    QPACK_STATIC_TABLE_ENTRY( 77, FIELD::Access_Control_Allow_Methods, "get, post, options" ) \
    QPACK_STATIC_TABLE_ENTRY( 78, FIELD::Access_Control_Allow_Methods, "options" ) \
    QPACK_STATIC_TABLE_ENTRY( 79, FIELD::Access_Control_Expose_Headers, "content-length" ) \
    QPACK_STATIC_TABLE_ENTRY( 80, FIELD::Access_Control_Request_Headers, "content-type" ) \
    QPACK_STATIC_TABLE_ENTRY( 81, FIELD::Access_Control_Request_Method, "get" ) \
    QPACK_STATIC_TABLE_ENTRY( 82, FIELD::Access_Control_Request_Method, "post" ) \
    QPACK_STATIC_TABLE_ENTRY( 83, FIELD::Alt_Svc, "clear" ) \
    QPACK_STATIC_TABLE_ENTRY( 84, FIELD::Authorization, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 85, FIELD::Content_Security_Policy, "script-src 'none'; object-src 'none'; base-uri 'none'" ) \
    QPACK_STATIC_TABLE_ENTRY( 86, FIELD::Early_Data, "1" ) \
    QPACK_STATIC_TABLE_ENTRY( 87, FIELD::Expect_CT, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 88, FIELD::Forwarded, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 89, FIELD::If_Range, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 90, FIELD::Origin, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 91, FIELD::Purpose, "prefetch" ) \
    QPACK_STATIC_TABLE_ENTRY( 92, FIELD::Server, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 93, FIELD::Timing_Allow_Origin, "*" ) \
    QPACK_STATIC_TABLE_ENTRY( 94, FIELD::Upgrade_Insecure_Requests, "1" ) \
    QPACK_STATIC_TABLE_ENTRY( 95, FIELD::User_Agent, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 96, FIELD::X_Forwarded_For, "" ) \
    QPACK_STATIC_TABLE_ENTRY( 97, FIELD::X_Frame_Options, "deny" ) \
    QPACK_STATIC_TABLE_ENTRY( 98, FIELD::X_Frame_Options, "sameorigin" ) \
    LIST_DEFINITION_END

namespace header {

    constexpr void parse(const Stream &stream, hpack::dynamic_table_t &dynamic_table, std::function<void(const std::pair<FIELD, std::string> &)> add_field) {
        if ((*stream & 0x80)) { // 4.5.2. https://www.rfc-editor.org/rfc/rfc9204.html#name-indexed-field-line
            // TODO: Check for voilation
            const bool T = *stream & 0x40;
            uint32_t index = hpack::decode_integer<6>(stream);
            auto &header = T ? static_table[index] : dynamic_table[index];
            add_field(header);
        } else if ((*stream & 0xf0) == 0x10) { // 4.5.3.  https://www.rfc-editor.org/rfc/rfc9204.html#name-indexed-field-line-with-pos
            uint32_t index = hpack::decode_integer<4>(stream);
            auto &header = dynamic_table[index];
            add_field(header);
        } else
        // *stream cannot have 8th bit set, hence 0x40 check is sufficient
        if ((*stream & 0x40)) { // 4.5.4. https://www.rfc-editor.org/rfc/rfc9204.html#name-literal-field-line-with-nam
            const bool N = *stream & 0x20;
            const bool T = *stream & 0x10;
            uint32_t index = hpack::decode_integer<4>(stream);
            auto value = hpack::get_header_string<8>(stream);
            auto &header_indexed = T ? static_table[index] : dynamic_table[index];
            std::pair<FIELD, std::string> header { header_indexed.first, value };
            // TODO: move header
            if (N) dynamic_table.insert(header);
            add_field(header);
        } else if (!(*stream & 0xf0)) { // 4.5.5. https://www.rfc-editor.org/rfc/rfc9204.html#name-literal-field-line-with-pos
            const bool N = *stream & 0x08;
            uint32_t index = hpack::decode_integer<3>(stream);
            auto value = hpack::get_header_string<8>(stream);
            auto &header_indexed = dynamic_table[index];
            std::pair<FIELD, std::string> header { header_indexed.first, value };
            // TODO: move header
            if (N) dynamic_table.insert(header);
            add_field(header);
        } else
        // *stream cannot have 8th and 7th bit set, hence 0x20 check is sufficient
        if ((*stream & 0x20)) { // 4.5.6. https://www.rfc-editor.org/rfc/rfc9204.html#name-literal-field-line-with-lit
            const bool N = *stream & 0x10;
            auto name = hpack::get_header_string<4>(stream);
            auto value = hpack::get_header_string<8>(stream);
            auto field = to_field(name);
            std::pair<FIELD, std::string> header { field, value };
            // TODO: move header
            if (N) dynamic_table.insert(header);
            add_field(header);
        }
    }
} // namespace header

} // namespace rohit::http::v2