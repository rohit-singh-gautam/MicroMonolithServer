/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/net/tcpserverevent.h>

namespace MMS::server {

class echo_t : public MMS::event::protocol_implementation_t {
public:
    void ProcessRead(const uint8_t *buffer, const size_t size, MMS::event::writer_t &writer) override {
        writer.Write(event::write_entry_const {buffer, size});
    }
};

class echocreator_t : public MMS::event::protocol_implementation_creator_t {

public:

    MMS::event::protocol_implementation_t *create_protocol_implementation() override {
        return new echo_t();
    }

};


};