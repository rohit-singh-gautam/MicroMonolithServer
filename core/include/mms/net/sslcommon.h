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
#include <openssl/ssl.h>

namespace MMS::net::ssl {

class common {
    static bool initialized;
    SSL_CTX *ctx { };

public:
    common(const char *cert, const char *private_key);
    ~common();
    common(const common &) = delete;
    common &operator=(const common &) = delete;

    auto GetContext() { return ctx; }
    const auto GetContext() const { return ctx; }
};

} // namespace MMS::net::ssl