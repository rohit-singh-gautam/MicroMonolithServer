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

#include <errno.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdint>
#include <fcntl.h>
#include <cstring>

namespace MMS {

#ifndef LIST_DEFINITION_END
#define LIST_DEFINITION_END
#endif

#ifdef ERROR_T_LIST
    #undef ERROR_T_LIST
#endif

#define ERROR_T_LIST \
    ERROR_T_ENTRY(SUCCESS, "SUCCESS") \
    ERROR_T_ENTRY(FAILURE, "FAILURE") \
    ERROR_T_ENTRY(SUCCESS_NONBLOCKING, "Call is non blocking") \
    ERROR_T_ENTRY(INITIATE_CLOSE, "This will initiate closure for class that returns this. This can be used in multiple way and is implementation dependent") \
    ERROR_T_ENTRY(SOCKET_CONNECT_ALREADY_CONNECTED, "Socket is already connected") \
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
    ERROR_T_ENTRY(MEMORY_ALLOCATION_FAILURE, "Unable to allocated memory, insufficient memory or high memory fragmentation, restart recommended") \
    \
    ERROR_T_ENTRY(STREAM_UNDERFLOW, "Stream moved before begin") \
    ERROR_T_ENTRY(STREAM_OVERFLOW, "Stream moved to end") \
    ERROR_T_ENTRY(STREAM_UNSUPPORTED_DATATYPE, "Attempt to write unsupported data type to stream") \
    \
    ERROR_T_ENTRY(SIGNAL_POLLING_FAILED, "Signal polling failed") \
    ERROR_T_ENTRY(SIGNAL_READ_FAILED, "Signal read failed") \
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
    \
    ERROR_T_ENTRY(SOCKET_SSL_CERTIFICATE_FILE_NOT_FOUND, "Failed to load SSL certificate, file not found") \
    ERROR_T_ENTRY(SOCKET_SSL_PRIKEY_FILE_NOT_FOUND, "Failed to load SSL private key, file not found") \
    \
    ERROR_T_ENTRY(SOCKOPT_FAILURE, "Unable to set socket option") \
    ERROR_T_ENTRY(SOCKOPT_BAD_ID, "Unable to set socket option, bad socket ID") \
    ERROR_T_ENTRY(SOCKOPT_UNKNOWN_OPTION, "Unknown socket option") \
    \
    ERROR_T_ENTRY(LOG_READ_FAILURE, "Unable to read log") \
    ERROR_T_ENTRY(LOG_FILE_OPEN_FAILURE, "Unable to open log file") \
    \
    ERROR_T_ENTRY(LISTENER_CREATE_FAILED, "Listener creation failed") \
    ERROR_T_ENTRY(LISTENER_ALREADY_CREATE_FAILED, "Listener can have only one instance. Second instance to create listener will fail") \
    \
    ERROR_T_ENTRY(LISTNER_EVENT_ADD_FAILED, "Event creation failed") \
    ERROR_T_ENTRY(LISTNER_EVENT_ENABLE_FAILED, "Event enable failed") \
    ERROR_T_ENTRY(LISTENER_CREATE_FAILED_ZERO, "Event creation failed for 0 file descriptor value") \
    ERROR_T_ENTRY(LISTNER_EVENT_REMOVE_FAILED, "Event remove failed") \
    ERROR_T_ENTRY(LISTENER_TERMINATE_THREAD, "Listener will stop loop hence terminate the thread. All allocation must be RAII for this to be successful.") \
    \
    ERROR_T_ENTRY(HPACK_TABLE_OUT_OF_RANGE, "HPACK table index out of range") \
    \
    ERROR_T_ENTRY(HTTP2_HPACK_TABLE_ERROR, "HTTP 2 HPACK internal error") \
    ERROR_T_ENTRY(HTTP2_INITIATE_GOAWAY, "HTTP 2 goaway initiated") \
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
    ERROR_T_ENTRY(HTTP11_PARSER_FAILURE, "HTTP 1.1 parser failed") \
    \
    ERROR_T_ENTRY(QUIC_ENCODE_INTEGER_FAILED, "Quick unable to encode given integer value") \
    \
    ERROR_T_ENTRY(NOT_FOUND, "Not Found") \
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

class FullStream;

class http_parser_failed_t : public exception_t {
    const FullStream &stream;

public:
    constexpr http_parser_failed_t(const FullStream &stream) 
        : exception_t(err_t::HTTP11_PARSER_FAILURE), stream { stream } { }

    std::string to_string();
};

class listener_create_failed_t : public exception_t {
protected:
    using exception_t::exception_t;

public:
    constexpr listener_create_failed_t() : exception_t { err_t::LISTENER_CREATE_FAILED } { }
};

class listener_already_created_t : public listener_create_failed_t {
public:
    constexpr listener_already_created_t() : listener_create_failed_t { err_t::LISTENER_ALREADY_CREATE_FAILED } { }


};

class signal_polling_failed_t : public exception_t {
public:
    constexpr signal_polling_failed_t() : exception_t { err_t::SIGNAL_POLLING_FAILED } { }
};

class signal_read_failed_t : public exception_t {
public:
    constexpr signal_read_failed_t() : exception_t { err_t::SIGNAL_READ_FAILED } { }
};

class listener_terminate_thread_t : public exception_t {
public:
    constexpr listener_terminate_thread_t() : exception_t { err_t::LISTENER_TERMINATE_THREAD } { }
};

class socket_fail_t : public exception_t {
public:
    using exception_t::exception_t;
};

class setsockopt_fail_t : public socket_fail_t {
public:
    constexpr setsockopt_fail_t(const err_t err) : socket_fail_t { err } { }
};

class accept_fail_t : public socket_fail_t {
public:
    constexpr accept_fail_t() : socket_fail_t { err_t::ACCEPT_FAILURE } { }
};

class bind_fail_t : public socket_fail_t {
    const int port;
public:
    constexpr bind_fail_t(const int port) : socket_fail_t { err_t::BIND_FAILURE }, port { port } { }
    std::string to_string() {
        std::string ret { "Bind failed for port: "};
        ret += std::to_string(port);
        ret += ", this port may be already in used on a privileged port, enable this app to run on privileged. Also, stop any application runing on same port";
        return ret;
    }
};

class listen_fail_t : public socket_fail_t {
public:
    constexpr listen_fail_t() : socket_fail_t { err_t::LISTEN_FAILURE } { }
};

class MemoryAllocationException : public exception_t {
public:
    constexpr MemoryAllocationException() : exception_t { err_t::MEMORY_ALLOCATION_FAILURE } { }
};

class StreamException : public exception_t {
public:
    using exception_t::exception_t;
};

class StreamUnderflowException : public StreamException {
public:
    StreamUnderflowException() : StreamException { err_t::STREAM_UNDERFLOW } { }
};

class StreamOverflowException : public StreamException {
public:
    StreamOverflowException() : StreamException { err_t::STREAM_OVERFLOW } { }
};

class StreamUnsupportedDataType : public StreamException {
public:
    StreamUnsupportedDataType() : StreamException { err_t::STREAM_UNSUPPORTED_DATATYPE } { }
};

class QUICEncodeIntgegerFailed: public exception_t {
public:
    constexpr QUICEncodeIntgegerFailed() : exception_t { err_t::QUIC_ENCODE_INTEGER_FAILED } { }
};

class HTTPException : public exception_t {
public:
    using exception_t::exception_t;
};

class HPACKException : public HTTPException {
public:
    using HTTPException::HTTPException;
};

} // namespace MMS