/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/net/base.h>

namespace MMS::server {

class echo_t : public net::protocol_t {
public:
    using net::protocol_t::protocol_t;

    void ProcessRead(const ConstStream &stream) override {
        Write(listener::write_entry_const {stream.curr(), stream.remaining_buffer()});
    }
};

class echocreator_t : public net::protocol_creator_t {

public:

    net::protocol_t *create_protocol() override {
        return new echo_t();
    }

};


};