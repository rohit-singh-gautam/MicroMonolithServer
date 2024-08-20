/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/log/log.h>
#include <mms/base/error.h>
#include <mms/net/sslcommon.h>

namespace MMS::net::ssl {

bool common::initialized { false };
common::common(const char *cert, const char *private_key) {
    if (!std::filesystem::exists(cert)) {
        log<log_t::SOCKET_SSL_CERT_LOAD_FAILED_FILE_NOT_FOUND>();
        throw exception_t(err_t::SOCKET_SSL_CERTIFICATE_FILE_NOT_FOUND);
    }
    if (!initialized) SSL_library_init();
    ctx = SSL_CTX_new(TLS_server_method());
    SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, private_key, SSL_FILETYPE_PEM);
    log<log_t::SOCKET_SSL_INITIALIZE>();
    SSL_load_error_strings();	
    OpenSSL_add_all_algorithms();
}

common::~common() {
    SSL_CTX_free(ctx);
}

} // namespace MMS::net::ssl