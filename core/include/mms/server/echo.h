/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/net/base.h>

namespace MMS::server {
// Defined by RFC: https://datatracker.ietf.org/doc/html/rfc862
class echo_t : public net::protocol_t {
public:
    using net::protocol_t::protocol_t;

    void ProcessRead(const Stream &stream) override {
        Write(stream);
    }
};

class echocreator_t : public net::protocol_creator_t {

public:

    net::protocol_t *create_protocol(int, const std::string_view &) override {
        return new echo_t();
    }

};


};