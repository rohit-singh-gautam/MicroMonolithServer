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
    void ProcessRead(const uint8_t *buffer, const size_t size, listener::writer_t &writer) override {
        writer.Write(listener::write_entry_const {buffer, size});
    }
};

class echocreator_t : public net::protocol_creator_t {

public:

    net::protocol_t *create_protocol() override {
        return new echo_t();
    }

};


};