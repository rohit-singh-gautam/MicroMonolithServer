/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <errno.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>

namespace rohit {

#ifndef LIST_DEFINITION_END
#define LIST_DEFINITION_END
#endif

#define ERROR_T_LIST \
    ERROR_T_ENTRY(SUCCESS, "SUCCESS") \
    ERROR_T_ENTRY(SUCCESS_NONBLOCKING, "Call is non blocking") \
    ERROR_T_ENTRY(SOCKET_CONNECT_ALREADY_CONNECTED, "Socket is already connected") \
    ERROR_T_ENTRY(GENERAL_FAILURE, "Failed") \
    ERROR_T_ENTRY(CRITICAL_FAILURE, "Critical failure such as no memory server must be restarted") \
    ERROR_T_ENTRY(ORDERLY_SHUTDOWN, "Peer has performed orderly shutdown") \
    ERROR_T_ENTRY(CONNECTION_REFUSED, "Peer connection is closed") \
    ERROR_T_ENTRY(BIND_FAILURE, "Unable to bind to a socket") \
    ERROR_T_ENTRY(LISTEN_FAILURE, "Socket is already connected") \
    ERROR_T_ENTRY(ACCEPT_FAILURE, "Unable to accept connection to a socket") \
    ERROR_T_ENTRY(CLOSE_FAILURE, "Unable to close a socket") \
    ERROR_T_ENTRY(RECEIVE_FAILURE, "Unable to read from a socket") \
    ERROR_T_ENTRY(SEND_FAILURE, "Unable to write to a socket") \
    ERROR_T_ENTRY(BAD_FILE_DESCRIPTOR, "Bad file descriptor") \
    \
    ERROR_T_ENTRY(MAPPING_NOT_FOUND, "General mapping error") \
    \
    ERROR_T_ENTRY(PTHREAD_JOIN_FAILURE, "Unable to join to a thread") \
    ERROR_T_ENTRY(PTHREAD_JOIN_DEADLOCK_FAILURE, "pthread_join created a deadlock") \
    ERROR_T_ENTRY(PTHREAD_JOIN_NOT_JOINABLE_FAILURE, "Either thread is not joinable probably other thread is trying to join") \
    ERROR_T_ENTRY(PTHREAD_JOIN_INVALID_THREAD_ID_FAILURE, "No thread is running with given thread ID") \
    \
    ERROR_T_ENTRY(PTHREAD_CREATE_FAILURE, "Unable to create thread") \
    \
    ERROR_T_ENTRY(GUID_BAD_STRING_FAILURE, "Bad GUID string") \
    \
    ERROR_T_ENTRY(MESSAGE_COMMAND_LIMIT_FAILURE, "Cannot add more command to message") \
    \
    ERROR_T_ENTRY(SOCKET_FAILURE, "Unable to create socket") \
    ERROR_T_ENTRY(SOCKET_PERMISSION_FAILURE, "Insufficient permission to create socket") \
    ERROR_T_ENTRY(SOCKET_ADDRESS_NOT_SUPPORTED, "Address type is not supported") \
    ERROR_T_ENTRY(SOCKET_PROTOCOL_NOT_SUPPORTED, "Protocol type is not supported") \
    ERROR_T_ENTRY(SOCKET_LIMIT_REACHED, "Filedescriptor limit for socket reached") \
    ERROR_T_ENTRY(SOCKET_INSUFFICIENT_MEMORY, "Insufficient memory") \
    \
    ERROR_T_ENTRY(SOCKET_CONNECT_FAILURE, "Unable to connect to socket") \
    ERROR_T_ENTRY(SOCKET_CONNECT_PERMISSION_FAILURE, "Insufficient permission to connect to socket") \
    ERROR_T_ENTRY(SOCKET_CONNECT_ADDRESS_IN_USE, "Already connected with this address") \
    ERROR_T_ENTRY(SOCKET_CONNECT_ADDRESS_NOT_SUPPORTED, "Address type is not supported") \
    ERROR_T_ENTRY(SOCKET_CONNECT_INVALID_ID, "Invalid socket ID") \
    ERROR_T_ENTRY(SOCKET_CONNECT_CONNECTION_REFUSED, "Connection refused, remote server not accepting connection") \
    ERROR_T_ENTRY(SOCKET_CONNECT_INTERRUPTED, "Connect was interrupted due to signal") \
    ERROR_T_ENTRY(SOCKET_CONNECT_NETWORK_UNREACHABLE, "Unreachable server") \
    ERROR_T_ENTRY(SOCKET_CONNECT_UNSUPPORTED_PROTOCOL, "Unsupported protocol") \
    ERROR_T_ENTRY(SOCKET_CONNECT_TIMEOUT, "Unable to connect as it timeout") \
    ERROR_T_ENTRY(SOCKET_WRITE_ZERO, "Socket write written zero byte") \
    ERROR_T_ENTRY(SOCKET_RETRY, "Socket retry last operation") \
    ERROR_T_ENTRY(SOCKET_GET_READ_BUFFER_FAILED, "Socket read buffer get failed") \
    ERROR_T_ENTRY(SOCKET_GET_WRITE_BUFFER_FAILED, "Socket write buffer get failed") \
    ERROR_T_ENTRY(SOCKET_SET_READ_BUFFER_FAILED, "Socket read buffer set failed") \
    ERROR_T_ENTRY(SOCKET_SET_WRITE_BUFFER_FAILED, "Socket write buffer set failed") \
    \
    ERROR_T_ENTRY(SOCKET_SSL_CONTEXT_FAILED, "Creation on SSL context failed") \
    ERROR_T_ENTRY(SOCKET_SSL_CERTIFICATE_FAILED, "Failed to load SSL certificate") \
    ERROR_T_ENTRY(SOCKET_SSL_CERTIFICATE_FILE_NOT_FOUND, "Failed to load SSL certificate, file not found") \
    ERROR_T_ENTRY(SOCKET_SSL_PRIKEY_FAILED, "Failed to load Primay Key") \
    \
    ERROR_T_ENTRY(SOCKET_SSL_WANT_READ, "Non blocking SSL, want to read more data") \
    ERROR_T_ENTRY(SOCKET_SSL_WANT_WRITE, "Non blocking SSL, want to write more data") \
    ERROR_T_ENTRY(SOCKET_SSL_WANT_X509_LOOKUP, "Non blocking SSL, looking for certificate") \
    ERROR_T_ENTRY(SOCKET_SSL_SYSCALL_ERROR, "SSL syscall error, probably connection is closed or wrong parameter") \
    ERROR_T_ENTRY(SOCKET_SSL_ZERO_RETURN, "SSL zero bytes returned no data") \
    ERROR_T_ENTRY(SOCKET_SSL_ERROR, "SSL error, Openssl call failed") \
    ERROR_T_ENTRY(SOCKET_SSL_SANITY_ERROR, "SSL error, parameter changes requires exactly same parameter") \
    \
    ERROR_T_ENTRY(SOCKOPT_FAILURE, "Unable to set socket option") \
    ERROR_T_ENTRY(SOCKOPT_BAD_ID, "Unable to set socket option, bad socket ID") \
    ERROR_T_ENTRY(SOCKOPT_UNKNOWN_OPTION, "Unknown socket option") \
    \
    ERROR_T_ENTRY(LOG_READ_FAILURE, "Unable to read log") \
    ERROR_T_ENTRY(LOG_UNSUPPORTED_TYPE_FAILURE, "Unsupported log type") \
    ERROR_T_ENTRY(LOG_FILE_OPEN_FAILURE, "Unable to open log file") \
    \
    ERROR_T_ENTRY(MATH_INSUFFICIENT_BUFFER, "Buffer is not sufficient to store result, partial and wrong result may have been written to buffer") \
    \
    ERROR_T_ENTRY(EVENT_DIST_CREATE_FAILED, "Event distributor creation failed") \
    ERROR_T_ENTRY(EVENT_CREATE_FAILED, "Event creation failed") \
    ERROR_T_ENTRY(EVENT_ENABLE_FAILED, "Event enable failed") \
    ERROR_T_ENTRY(EVENT_CREATE_FAILED_ZERO, "Event creation failed for 0 file descriptor value") \
    ERROR_T_ENTRY(EVENT_REMOVE_FAILED, "Event remove failed") \
    \
    ERROR_T_ENTRY(FILEWATCHER_ADD_FOLDER_FAILED, "FILEWATCHER failed to add folder for watch") \
    ERROR_T_ENTRY(FILEWATCHER_EVENT_CREATE_FAILED, "FILEWATCHER event create failed") \
    \
    ERROR_T_ENTRY(SSL_CONNECT_FAILED, "Failed to create SSL session") \
    ERROR_T_ENTRY(SSL_SESSION_NULL, "SSL session in NULL") \
    ERROR_T_ENTRY(CRYPTO_MEMORY_BAD_ASSIGNMENT, "Assigning to non null memory, make sure to free it first") \
    ERROR_T_ENTRY(CRYPTO_UNKNOWN_ALGORITHM, "Unknown crypto algorithm") \
    ERROR_T_ENTRY(CRYPTO_MEMORY_FAILURE, "Failed to allocated OpenSSL memory") \
    ERROR_T_ENTRY(CRYPTO_CREATE_CONTEXT_FAILED, "Failed to create OpenSSL encryption/decryption context") \
    ERROR_T_ENTRY(CRYPTO_INIT_AES_FAILED, "Failed to initialize OpenSSL AES encryption/decryption") \
    ERROR_T_ENTRY(CRYPTO_ENCRYPT_AES_FAILED, "Failed OpenSSL AES encryption") \
    ERROR_T_ENTRY(CRYPTO_DECRYPT_AES_FAILED, "Failed OpenSSL AES decryption") \
    ERROR_T_ENTRY(CRYPTO_KEY_GENERATION_FAILED, "Failed OpenSSL to generate key") \
    ERROR_T_ENTRY(CRYPTO_CONTEXT_CREATION_FAILED, "Context creation failed in OpenSSL") \
    ERROR_T_ENTRY(CRYPTO_BAD_KEY, "Bad key provided") \
    ERROR_T_ENTRY(CRYPTO_BAD_PUBLIC_KEY, "Bad public key provided") \
    ERROR_T_ENTRY(CRYPTO_BAD_PRIVATE_KEY, "Bad private key provided") \
    ERROR_T_ENTRY(CRYPTO_BAD_SYMETRIC_KEY, "Bad symetric key provided") \
    ERROR_T_ENTRY(CRYPTO_KEY_ENCODE_FAIL, "Failed to encode key to binary") \
    ERROR_T_ENTRY(CRYPTO_CURVE_NOT_FOUND, "Failed to find curve from key") \
    \
    ERROR_T_ENTRY(HTTP_FILEMAP_NOT_FOUND, "Failed to find filemap") \
    ERROR_T_ENTRY(HTTP11_PARSER_MEMORY_FAILURE, "HTTP 1.1 parser failed to allocate memory") \
    ERROR_T_ENTRY(HTTP11_PARSER_FAILURE, "HTTP 1.1 parser failed") \
    \
    ERROR_T_ENTRY(HTTP2_HPACK_TABLE_ERROR, "HTTP 2 HPACK internal error") \
    ERROR_T_ENTRY(HTTP2_INITIATE_GOAWAY, "HTTP 2 goaway initiated") \
    \
    ERROR_T_ENTRY(MAX_FAILURE, "Max failure nothing beyond this") \
    LIST_DEFINITION_END

enum class err_t : uint16_t {
#define ERROR_T_ENTRY(x, y) x,
        ERROR_T_LIST
#undef ERROR_T_ENTRY
};

constexpr err_t &operator++(err_t &err) { return err = static_cast<err_t>(static_cast<std::underlying_type_t<err_t>>(err) + 1); }
constexpr err_t operator++(err_t err, int) { 
    err_t reterr = err;
    err = static_cast<err_t>(static_cast<std::underlying_type_t<err_t>>(err) + 1);
    return reterr;
}

template <bool null_terminated = true>
constexpr size_t to_string_size(const err_t &val) {
    if constexpr (null_terminated) {
        constexpr size_t displaystr_size[] =  {
#define ERROR_T_ENTRY(x, y) sizeof(#x" - " y),
    ERROR_T_LIST
#undef ERROR_T_ENTRY
        };
        return displaystr_size[(size_t)val];
    } else {
        constexpr size_t displaystr_size[] =  {
#define ERROR_T_ENTRY(x, y) (sizeof(#x" - " y) - 1),
    ERROR_T_LIST
#undef ERROR_T_ENTRY
        };
        return displaystr_size[(size_t)val];
    }
}

constexpr bool isFailure(const err_t &err) {
    return err != err_t::SUCCESS;
}

constexpr const char *err_t_string[] = {
#define ERROR_T_ENTRY(x, y) {#x" - " y},
    ERROR_T_LIST
#undef ERROR_T_ENTRY
};

template <bool null_terminated = true>
constexpr size_t to_string(const err_t &val, char *dest) {
    auto len = to_string_size<null_terminated>(val);
    const char *errstr = err_t_string[(size_t)val];
    std::copy(errstr, errstr + len, dest);
    return len;
}

class error_helper_t {
protected:
    err_t value;

public:
    constexpr error_helper_t(const error_helper_t &err) : value(err.value) {}
    constexpr error_helper_t(const err_t value) : value(value) {}

    constexpr error_helper_t& operator=(const error_helper_t rhs) { value = rhs.value; return *this; }

    // Description will not be compared
    constexpr bool operator==(const error_helper_t rhs) const { return value == rhs; }
    constexpr bool operator!=(const error_helper_t rhs) const { return value != rhs; }
    constexpr bool operator==(const err_t rhs) const { return value == rhs; }
    constexpr bool operator!=(const err_t rhs) const { return value != rhs; }

    std::string to_string() const {
        auto len = to_string_size<false>(value);
        auto errstr = err_t_string[static_cast<size_t>(value)];
        return { errstr, len };
    }

    constexpr operator err_t() const { return value; }


    constexpr bool isSuccess() const { return value == err_t::SUCCESS; }
    constexpr bool isFailure() const { return value != err_t::SUCCESS; }

    static inline err_t socket_create_ret() {
        switch (errno)
        {
        case 0: return err_t::SUCCESS;
        case EACCES: return err_t::SOCKET_PERMISSION_FAILURE;
        case EAFNOSUPPORT: return err_t::SOCKET_ADDRESS_NOT_SUPPORTED;
        case EPROTONOSUPPORT:
        case EINVAL: return err_t::SOCKET_PROTOCOL_NOT_SUPPORTED;
        case EMFILE: return err_t::SOCKET_LIMIT_REACHED;
        case ENOBUFS:
        case ENOMEM: return err_t::SOCKET_INSUFFICIENT_MEMORY;
        default: return err_t::SOCKET_FAILURE;
        }
    }

    static inline err_t socket_connect_ret() {
        switch (errno)
        {
        case 0: return err_t::SUCCESS;
        case EAGAIN:
        case EALREADY:
        case EINPROGRESS: return err_t::SUCCESS_NONBLOCKING;
        case EISCONN: return err_t::SOCKET_CONNECT_ALREADY_CONNECTED;
        case EACCES:
        case EPERM: return err_t::SOCKET_CONNECT_PERMISSION_FAILURE;
        case EADDRINUSE: 
        case EADDRNOTAVAIL: return err_t::SOCKET_CONNECT_ADDRESS_IN_USE;
        case EAFNOSUPPORT: return err_t::SOCKET_CONNECT_ADDRESS_NOT_SUPPORTED;
        case EBADF: 
        case EFAULT:
        case ENOTSOCK: return err_t::SOCKET_CONNECT_INVALID_ID;
        case ECONNREFUSED: return err_t::SOCKET_CONNECT_CONNECTION_REFUSED;
        case EINTR: return err_t::SOCKET_CONNECT_INTERRUPTED;
        case ENETUNREACH: return err_t::SOCKET_CONNECT_NETWORK_UNREACHABLE;
        case EPROTOTYPE: return err_t::SOCKET_CONNECT_UNSUPPORTED_PROTOCOL;
        case ETIMEDOUT: return err_t::SOCKET_CONNECT_TIMEOUT;

        default: return err_t::SOCKET_CONNECT_FAILURE;
        }
    }

    static inline err_t sockopt_ret() {
        switch (errno) {
            case 0: return err_t::SUCCESS;
            case EBADF:
            case ENOTSOCK: return err_t::SOCKOPT_BAD_ID;
            case ENOPROTOOPT: return err_t::SOCKOPT_UNKNOWN_OPTION;
            case EINVAL:
            default: return err_t::SOCKOPT_FAILURE;
        }
    }

}; // class error_helper_t

inline std::ostream& operator<<(std::ostream& os, const err_t &error) {
    const error_helper_t errdata { error };
    return os << errdata.to_string();
}

class exception_t : public error_helper_t {
public:
    using error_helper_t::error_helper_t;

};

class critical_exception_t : public exception_t {
public:
    constexpr critical_exception_t() : exception_t(err_t::CRITICAL_FAILURE) { }
};

inline std::ostream& operator<<(std::ostream& os, const exception_t &error) {
    const error_helper_t errdata { error };
    return os << errdata.to_string();
}

class http_parser_failed_t : public exception_t {
    const char *const current_position;
    const size_t buffer_remaining;
public:
    constexpr http_parser_failed_t(const char *const current_position, const size_t buffer_remaining) 
        : exception_t(err_t::HTTP11_PARSER_FAILURE), current_position { current_position }, buffer_remaining { buffer_remaining } { }

    std::string to_string(const char *start_position) {
        auto errstr = error_helper_t::to_string() + ": ";

        if (current_position < start_position) {
            errstr += "Underflow - ";
        } else 
        {
            if (current_position - start_position >= 40 ){
                for(size_t index { 0 }; index < 16; ++index) {
                    auto current_ch = start_position[index];
                    if (current_ch >= 32 /* &&  current_ch <= 127 */) {
                        errstr.push_back(current_ch);
                    } else errstr.push_back('#');
                }
                errstr += " ... ";

                for(size_t index { 16 }; index; --index) {
                    auto current_ch = *(current_position - index);
                    if (current_ch >= 32 /* &&  current_ch <= 127 */) {
                        errstr.push_back(current_ch);
                    } else errstr.push_back('#');
                }
            } else {
                auto itr = start_position;
                while(itr < current_position) {
                    auto current_ch = *itr;
                    if (current_ch >= 32 /* &&  current_ch <= 127 */) {
                        errstr.push_back(current_ch);
                    } else errstr.push_back(current_ch);

                    ++itr;
                }
            }

            errstr += " <-- error is here -- ";
        }

        for(size_t index { 0 }; index < std::min(16UL, buffer_remaining); ++index) {
            auto current_ch = current_position[index];
            if (current_ch >= 32 /* &&  current_ch <= 127 */) {
                errstr.push_back(current_ch);
            } else errstr.push_back('#');
        }
        if (buffer_remaining > 16) {
            errstr += " ... more " + std::to_string(buffer_remaining - 16UL) + " characters.";
        }

        return errstr;
    }
};

} // namespace rohit