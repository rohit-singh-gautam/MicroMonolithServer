/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/net/tcpsslserver.h>

namespace MMS::net::tcp::ssl {

thread_local buffer_t connection_t::tempbuffer { };

connection_t::~connection_t() {
    SSL_free(ssl);
}

err_t connection_t::ProcessRead() {
    size_t offset { 0 };
    while(true) {
        auto [buffer, buffer_size] = tempbuffer.GetBuffer(offset);
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
                // TODO: HOW TO HANDLE this
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
            offset += actualread;
            break;
        }
    }

EXIT_LOOP:
    if (offset) {
        protocol_implementation->ProcessRead(tempbuffer.GetBuffer<uint8_t *>(), offset, *this);
        log<log_t::TCP_CONNECTION_READ>(GetFD(), offset);
    }
    else log<log_t::TCP_CONNECTION_EMPTY_READ>(GetFD());

    return pending_wirte.empty() ? err_t::SUCCESS : err_t::SOCKET_RETRY;
}

err_t connection_t::ProcessWrite() {
    while(!pending_wirte.empty()) {
        auto &currentbuffer = pending_wirte.front();
        while(!currentbuffer.Completed()) {
            auto [buffer, size] = currentbuffer.GetBuffer<void *>();
            size_t actualwritten { };
            auto ret = SSL_write_ex(ssl, buffer, size, &actualwritten);
            if (!ret) {
                auto ssl_error = SSL_get_error(ssl, ret);
                switch(ssl_error) {
                case SSL_ERROR_ZERO_RETURN:
                    return err_t::ORDERLY_SHUTDOWN;

                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                    // TODO: HOW TO HANDLE this
                    goto EXIT_LOOP;

                default:
                    // Close will take care of termination
                    log<log_t::TCP_SERVER_PEER_WRITE_FAILED>(GetFD(), errno);
                    Close();
                    return err_t::BAD_FILE_DESCRIPTOR;
                }
            }
            currentbuffer.AddOffset(static_cast<size_t>(actualwritten));
            if (static_cast<size_t>(actualwritten) < size)  return err_t::SOCKET_RETRY;
        }
EXIT_LOOP:
        if (!currentbuffer.Completed()) break;
        pending_wirte.pop();
    }
    return err_t::SUCCESS;
}

void connection_t::WriteNoCopy(uint8_t* buffer, size_t bytesize, size_t byteoffset) {
    pending_wirte.emplace(buffer, bytesize, byteoffset);
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

    auto protocol = protocol_creator.create_protocol();
    auto connection = new connection_t(peer_id, ssl, protocol);
    auto ret = listener->add(connection);
    if (ret == err_t::SUCCESS) {
        log<log_t::TCP_SERVER_PEER_CREATED>(GetFD(), connection->GetFD(), connection->get_peer_ipv6_addr());
    } else {
        log<log_t::TCP_SERVER_PEER_CREATED>(GetFD(), connection->GetFD(), connection->get_peer_ipv6_addr());
        delete connection;
    }

    return err_t::SUCCESS;
}

} // namespace MMS::net::tcp::ssl