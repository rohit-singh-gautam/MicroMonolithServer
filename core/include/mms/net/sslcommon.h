/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

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