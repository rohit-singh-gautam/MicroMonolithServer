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

#include <mms/net/tcpsslserver.h>

namespace MMS::net::tcp::ssl {

connection_t::~connection_t() {
    SSL_free(ssl);
}

const std::string_view server_t::get_protocol(SSL *ssl) {
    const uint8_t *data;
    unsigned int len;
    SSL_get0_alpn_selected(ssl, &data, &len);

    if (data == nullptr) {
        SSL_get0_next_proto_negotiated(ssl, &data, &len);
    }
    return std::string_view { reinterpret_cast<const char *>(data), len };
}

err_t connection_t::ProcessRead() {
    while(true) {
        auto [buffer, buffer_size] = readbuffer.GetRawCurrentBuffer();
        size_t actualread { };
        auto ret = SSL_read_ex(ssl, buffer, buffer_size, &actualread);
        switch(ret) {
        case -1: {
            auto ssl_error = SSL_get_error(ssl, ret);
            switch(ssl_error) {
            case SSL_ERROR_ZERO_RETURN:
                return err_t::ORDERLY_SHUTDOWN;

            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                goto EXIT_LOOP;

            default: 
                log<log_t::TCP_SERVER_PEER_READ_FAILED>(GetFD(), errno);
                // Close will take care of termination
                Close();
                return err_t::BAD_FILE_DESCRIPTOR;
            }
            break;
        }
        case 0:
        default:
            if (actualread == 0) goto EXIT_LOOP;
            readbuffer += actualread;
            break;
        }
    }

EXIT_LOOP:
    if (readbuffer.index()) {
        protocol_implementation->ProcessRead(make_const_stream(readbuffer.begin(), readbuffer.curr()));
        log<log_t::TCP_CONNECTION_READ>(GetFD(), readbuffer.index());
    }
    else log<log_t::TCP_CONNECTION_EMPTY_READ>(GetFD());

    return err_t::SUCCESS;
}

err_t connection_t::ProcessWrite() {
    while(!pending_wirte.empty()) {
        auto &currentbuffer = pending_wirte.front();
        while(writeoffset != currentbuffer.size()) {
            const auto buffer = currentbuffer.begin() + writeoffset;
            size_t size = currentbuffer.size() - writeoffset;
            size_t actualwritten { };
            auto ret = SSL_write_ex(ssl, buffer, size, &actualwritten);
            if (!ret) {
                auto ssl_error = SSL_get_error(ssl, ret);
                switch(ssl_error) {
                case SSL_ERROR_ZERO_RETURN:
                    return err_t::ORDERLY_SHUTDOWN;

                case SSL_ERROR_WANT_READ:
                    return err_t::SUCCESS;

                case SSL_ERROR_WANT_WRITE:
                    return err_t::SOCKET_RETRY;

                default:
                    // Close will take care of termination
                    log<log_t::TCP_SERVER_PEER_WRITE_FAILED>(GetFD(), errno);
                    Close();
                    return err_t::BAD_FILE_DESCRIPTOR;
                }
            }
            writeoffset += actualwritten;
            if (static_cast<size_t>(actualwritten) < size)  return err_t::SOCKET_RETRY;
        }

        writeoffset = 0;
        pending_wirte.pop();
    }
    return err_t::SUCCESS;
}

err_t server_t::ProcessRead() {
    log<log_t::TCP_SERVER_RECEIVED_EVENT>(GetFD());

    auto peer_id = ::accept4(GetFD(), NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (peer_id == -1) {
        if (errno != EAGAIN) {
            log<log_t::TCP_SERVER_ACCEPT_FAILED>(GetFD(), errno);
        }
        return err_t::SUCCESS;
    }
    log<log_t::TCP_SOCKET_ACCEPT_SUCCESS>(GetFD(), peer_id);

    SSL *ssl = SSL_new(ssl_common->GetContext());
    if (ssl == nullptr) {
        log<log_t::TCP_SSL_CREATION_FAILED>(GetFD(), peer_id);
        close(peer_id);
        // Will be returing from here
        return err_t::SUCCESS;
    }
    auto ssl_ret = SSL_set_fd(ssl, peer_id);
    if (ssl_ret < 0) {
        auto ssl_error = SSL_get_error(ssl, ssl_ret);
        log<log_t::TCP_SSL_INITIALIZATION_FAILED>(GetFD(), peer_id, ssl_error);
        SSL_free(ssl);
        close(peer_id);
        return err_t::SUCCESS;
    }
    ssl_ret = SSL_accept(ssl);
    if (ssl_ret < 0) {
        auto ssl_error = SSL_get_error(ssl, ssl_ret);
        switch(ssl_error) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            break;

        case SSL_ERROR_SSL:
            log<log_t::TCP_SSL_ACCEPT_FAILED_NON_SSL>(GetFD(), peer_id);
            SSL_free(ssl);
            close(peer_id);
            return err_t::SUCCESS;

        default:
            log<log_t::TCP_SSL_ACCEPT_FAILED>(GetFD(), peer_id, ssl_error);
            SSL_free(ssl);
            close(peer_id);
            return err_t::SUCCESS;
        }
    }

    auto proto = get_protocol(ssl);

    auto protocol = protocol_creator.create_protocol(peer_id, proto);
    if (protocol) {
        auto connection = new connection_t(peer_id, ssl, protocol);
        protocol->SetProcessor(connection);
        auto ret = listener->add(connection);
        if (ret == err_t::SUCCESS) {
            log<log_t::TCP_SERVER_PEER_CREATED>(GetFD(), connection->GetFD(), connection->get_peer_ipv6_addr());
        } else {
            log<log_t::TCP_SERVER_PEER_CREATE_FAILED>(GetFD(), connection->GetFD(), connection->get_peer_ipv6_addr());
            delete connection;
        }
    } else {
        log<log_t::TCP_SERVER_PEER_CREATE_FAILED>(GetFD(), peer_id, get_peer_ipv6_addr(peer_id));
        close(peer_id);
        SSL_free(ssl);
    }

    // Return must always be success or server will be stopped.
    return err_t::SUCCESS;
}

} // namespace MMS::net::tcp::ssl