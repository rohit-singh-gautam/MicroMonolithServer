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