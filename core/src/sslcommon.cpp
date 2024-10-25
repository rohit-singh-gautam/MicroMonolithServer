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

#include <mms/log/log.h>
#include <mms/base/error.h>
#include <mms/net/sslcommon.h>

namespace MMS::net::ssl {

bool common::initialized { false };
common::common(const char *cert, const char *private_key) {
    if (!std::filesystem::exists(cert)) {
        log<log_t::SOCKET_SSL_CERTIFICATE_FILE_NOT_FOUND>();
        throw exception_t(err_t::SOCKET_SSL_CERTIFICATE_FILE_NOT_FOUND);
    }
    if (!std::filesystem::exists(cert)) {
        log<log_t::SOCKET_SSL_PRIKEY_FILE_NOT_FOUND>();
        throw exception_t(err_t::SOCKET_SSL_PRIKEY_FILE_NOT_FOUND);
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