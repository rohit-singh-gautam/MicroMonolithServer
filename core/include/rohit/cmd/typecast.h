/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <rohit/base/types.h>
#include <rohit/base/maths.h>
#include <rohit/base/guid.h>
#include <rohit/net/ipv6addr.h>
#include <rohit/log/varadic.h>

namespace rohit {

constexpr bool to_bool(const char *value) {
    return *value == 'T' || *value == 't';
}

constexpr bool to_type(const type_identifier id, void *store, const char *value) {
    switch (id) {    
    case type_identifier::bool_t:
        *(bool_t *)store = to_bool(value);
        break;
    
    case type_identifier::uint8_t:
        *(uint8_t *)store = to_uint<uint8_t>(value);
        break;

    case type_identifier::uint16_t:
        *(uint16_t *)store = to_uint<uint16_t>(value);
        break;

    case type_identifier::uint32_t:
        *(uint32_t *)store = to_uint<uint32_t>(value);
        break;

    case type_identifier::uint64_t:
        *(uint64_t *)store = to_uint<uint64_t>(value);
        break;

    case type_identifier::int8_t:
        *(int8_t *)store = to_int<int8_t>(value);
        break;

    case type_identifier::int16_t:
        *(int16_t *)store = to_int<int16_t>(value);
        break;

    case type_identifier::int32_t:
        *(int32_t *)store = to_int<int32_t>(value);
        break;

    case type_identifier::int64_t:
        *(int64_t *)store = to_int<int64_t>(value);
        break;

    case type_identifier::string_t:
        *(const char **)store = value;
        break;

    case type_identifier::guid_t:
        *(guid_t *)store = to_guid(value);
        break;

    case type_identifier::ipv6_addr_t:
        *(ipv6_addr_t *)store = to_ipv6_addr_t(value);
        break;

    case type_identifier::ipv6_port_t: {
            auto ipv6_port_val = to_ipv6_port_t(value);
            copyvaradic((uint8_t *)store, ipv6_port_val);
            break;
        }

    case type_identifier::ipv6_socket_addr_t: {
            auto ipv6_socket_val = to_ipv6_socket_addr_t(value);
            copyvaradic((uint8_t *)store, ipv6_socket_val);
            break;
        }

    case type_identifier::filepath_t: {
        *(std::filesystem::path *)store = std::filesystem::path(value);
        break;
    }

    case type_identifier::stdstring_t: {
        *(std::string *)store = std::string(value);
        break;
    }
        
    default:
        return false;

    }

    return true;
}


}