/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <unordered_map>
#include <string>
#include <cassert>
#include <cstdint>

namespace MMS {

#define LIST_DEFINITION_END

    // Supported format specifier
    // %d or %i - signed integer (int32_t)
    // %u - unsigned integer (uint32_t)
    // %o - unsigned integer (uint32_t)(Octal)
    // %x - unsigned integer (uint32_t)(hex small case)
    // %X - unsigned integer (uint32_t)(hex capital case)
    // %f - floating point lower case (float)
    // %F - floating point upper case (float)
    // %c - character (char)
    // %v - Custom
    //      %vn: IPv6 socket Address format
    //      %vN: IPv6 socket Address format in caps
    //      %vi: IPv6 address
    //      %vi: IPv6 address in caps
    //      %vp: IPv6 port
    //      %ve: System errno
    //      %vE: IOT error
    //      %vg: GUID lower case
    //      %vG: GUID upper case
    //      %vv: Epoll event
    //      %vc: SSL error
    //      %vm: Module name
    //      %vl: Log level
    // %% - %
    //
    // Supported format length
    // h - short - 16 bits
    // hh - ultra short 8 bits
    // l - long
    // ll - long long

#define LOGGER_LEVEL_LIST \
    LOGGER_LEVEL_ENTRY(IGNORE) \
    LOGGER_LEVEL_ENTRY(DEBUG) \
    LOGGER_LEVEL_ENTRY(VERBOSE) \
    LOGGER_LEVEL_ENTRY(INFO) \
    LOGGER_LEVEL_ENTRY(WARNING) \
    LOGGER_LEVEL_ENTRY(ERROR) \
    LOGGER_LEVEL_ENTRY(ALERT) \
    LIST_DEFINITION_END

#define LOGGER_MODULE_LIST \
    LOGGER_MODULE_ENTRY(SYSTEM) \
    LOGGER_MODULE_ENTRY(SOCKET) \
    LOGGER_MODULE_ENTRY(LISTENER) \
    LOGGER_MODULE_ENTRY(TCP_SERVER) \
    LOGGER_MODULE_ENTRY(IOT_HTTPSERVER) \
    LOGGER_MODULE_ENTRY(IOT_HTTP2SERVER) \
    LOGGER_MODULE_ENTRY(TEST) \
    LOGGER_MODULE_ENTRY(MAX_MODULE) \
    LOGGER_MODULE_ENTRY(UNKNOWN) \
    LIST_DEFINITION_END

#define LOGGER_LOG_LIST \
    LOGGER_ENTRY(BADLOG_ERROR, ALERT, SYSTEM, "Your log is corrupted, delete it, restart server and use latest log reader !!!!!!!!!!!!!!") \
    LOGGER_ENTRY(SEGMENTATION_FAULT, ALERT, SYSTEM, "Segmentation fault occurred !!!!!!!!!!!!!!") \
    \
    LOGGER_ENTRY(APPLICATION_STARTING, ALERT, SYSTEM, "Application is starting") \
    LOGGER_ENTRY(APPLICATION_STARTED_SUCCESSFULLY, ALERT, SYSTEM, "Application started successfully") \
    LOGGER_ENTRY(TCP_SOCKET_CREATE_SUCCESS, DEBUG, SOCKET, "TCP Socket %i created") \
    LOGGER_ENTRY(TCP_SOCKET_CREATE_FAILED, INFO, SOCKET, "TCP Socket creation failed with error %ve") \
    LOGGER_ENTRY(TCP_SOCKET_CLOSE_SUCCESS, DEBUG, SOCKET, "TCP Socket %i closed") \
    LOGGER_ENTRY(TCP_SOCKET_CLOSE_FAILED, INFO, SOCKET, "TCP Socket %i close failed with error %ve") \
    LOGGER_ENTRY(TCP_SOCKET_BIND_SUCCESS, DEBUG, SOCKET, "TCP Socket %i, port %i bind success") \
    LOGGER_ENTRY(TCP_SOCKET_LISTEN_SUCCESS, DEBUG, SOCKET, "TCP Socket %i, port %i listen success") \
    LOGGER_ENTRY(TCP_SOCKET_ACCEPT_SUCCESS, DEBUG, SOCKET, "TCP Socket %i accept success, new socket created %i") \
    LOGGER_ENTRY(SETTING_LOG_LEVEL_FAILED, ALERT, SYSTEM, "FAILED: Setting Log level %vl for module %vm") \
    LOGGER_ENTRY(SETTING_LOG_LEVEL, ALERT, SYSTEM, "Setting Log level %vl for module %vm") \
    LOGGER_ENTRY(SETTING_LOG_LEVEL_ALL, ALERT, SYSTEM, "Setting Log level %vl for all modules") \
    \
    LOGGER_ENTRY(SIGNAL_POLLING_MASK_FAILED, ALERT, SYSTEM, "Failed to set mask for SIGTERM and SIGINT with error %ve") \
    LOGGER_ENTRY(SIGNAL_READ_FAILED, ALERT, SYSTEM, "Failed to read signal for SIGTERM and SIGINT with error %ve") \
    LOGGER_ENTRY(SIGNAL_FD_FAILED, ALERT, SYSTEM, "Failed to get signalfd %ve") \
    \
    LOGGER_ENTRY(SOCKET_SSL_INITIALIZE, INFO, SOCKET, "Socket initialize SSL") \
    LOGGER_ENTRY(SOCKET_SSL_CERTIFICATE_FILE_NOT_FOUND, ERROR, SOCKET, "Unable to load SSL certificate as file not found, exiting") \
    LOGGER_ENTRY(SOCKET_SSL_PRIKEY_FILE_NOT_FOUND, ERROR, SOCKET, "Unable to load SSL private key as file not found, exiting") \
    \
    LOGGER_ENTRY(LISTENER_CREATE_FAILED, ERROR, LISTENER, "Listener creation failed with error %ve, terminating application") \
    LOGGER_ENTRY(LISTENER_ALREADY_CREATED_FAILED, ERROR, LISTENER, "Listener can have only one instance. Second instance to create listener will fail") \
    LOGGER_ENTRY(LISTENER_TERMINATION_HANDLER, ERROR, LISTENER, "Listener termination handler added successfully") \
    LOGGER_ENTRY(LISTENER_CREATING_THREAD, INFO, LISTENER, "Listener creating %llu threads") \
    LOGGER_ENTRY(LISTENER_EXITING_THREAD, INFO, LISTENER, "Listener exiting thread") \
    LOGGER_ENTRY(LISTENER_LOOP_CREATED, DEBUG, LISTENER, "Listener thread loop created") \
    LOGGER_ENTRY(LISTENER_LOOP_WAIT_INTERRUPTED, WARNING, LISTENER, "Listener loop interrupted with error %ve,  waiting for a second and retry") \
    LOGGER_ENTRY(LISTENER_TOO_MANY_THREAD, WARNING, LISTENER, "Listener created with threads more than CPUs, thread requestd %llu and number of CPU thread %llu") \
    LOGGER_ENTRY(LISTENER_EXIT_THREAD_JOIN_SUCCESS, VERBOSE, LISTENER, "Listener join thread success") \
    LOGGER_ENTRY(LISTENER_CREATE_SUCCESS, INFO, LISTENER, "Listener creation succeeded") \
    LOGGER_ENTRY(LISTENER_EVENT_RECEIVED, DEBUG, LISTENER, "Listener FD %i event %vv receive") \
    LOGGER_ENTRY(LISTNER_EVENT_ADD_FAILED, ERROR, LISTENER, "FD %i: Event creation failed with error %ve") \
    LOGGER_ENTRY(LISTNER_EVENT_ADD_SUCCESS, DEBUG, LISTENER, "FD %i: Event creation succeeded") \
    LOGGER_ENTRY(LISTNER_EVENT_REMOVE_SUCCESS, DEBUG, LISTENER, "FD %i: Event removal succeeded") \
    LOGGER_ENTRY(LISTNER_EVENT_REMOVE_FAILED, INFO, LISTENER, "FD %i: Event removal failed with error %ve") \
    LOGGER_ENTRY(LISTNER_EVENT_ENABLE_FAILED, ERROR, LISTENER, "FD %i: Event enable failed with error %ve") \
    LOGGER_ENTRY(LISTNER_EVENT_ENABLE_SUCCESS, DEBUG, LISTENER, "FD %i: Event enable succeeded") \
    \
    LOGGER_ENTRY(TCP_SERVER_RECEIVED_EVENT, DEBUG, TCP_SERVER, "FD %i: TCP server received event") \
    LOGGER_ENTRY(TCP_SERVER_ACCEPT_FAILED, ERROR, TCP_SERVER, "FD %i: TCP server failed to accept connection with error %ve") \
    LOGGER_ENTRY(TCP_SERVER_PEER_CREATED, VERBOSE, TCP_SERVER, "FD %i: TCP server new peer %i created from remote %vN") \
    LOGGER_ENTRY(TCP_SERVER_PEER_ADD_FAILED, WARNING, TCP_SERVER, "FD %i: TCP server peer %i add to listener failed %vN") \
    LOGGER_ENTRY(TCP_SERVER_PEER_WRITE_FAILED, WARNING, TCP_SERVER, "FD %i: TCP server peer write failed with error %ve") \
    LOGGER_ENTRY(TCP_SERVER_PEER_READ_FAILED, WARNING, TCP_SERVER, "FD %i: TCP server peer read failed with error %ve") \
    LOGGER_ENTRY(TCP_SERVER_PEER_CONNECTION_CLOSED, INFO, TCP_SERVER, "FD %i: TCP Server peer connection closed") \
    \
    LOGGER_ENTRY(TCP_CONNECTION_EMPTY_READ, DEBUG, TCP_SERVER, "FD %i: TCP empty read") \
    LOGGER_ENTRY(TCP_CONNECTION_READ, DEBUG, TCP_SERVER, "FD %i: TCP read %lu bytes") \
    \
    LOGGER_ENTRY(TCP_SSL_CREATION_FAILED, ERROR, TCP_SERVER, "FD %i: SSL TCP Server is unable to create SSL for peer %i") \
    LOGGER_ENTRY(TCP_SSL_INITIALIZATION_FAILED, ERROR, TCP_SERVER, "FD %i: SSL TCP Server unable to initialize peer %i, failed with error %vc") \
    LOGGER_ENTRY(TCP_SSL_ACCEPT_FAILED, ERROR, TCP_SERVER, "FD %i: SSL TCP Server unable to accept peer %i, failed with error %vc") \
    LOGGER_ENTRY(TCP_SSL_ACCEPT_FAILED_NON_SSL, ERROR, TCP_SERVER, "FD %i: SSL TCP Server peer %i, failed as non SSL protocol used. Protocol like https must be used, http will not connect to SSL server.") \
    \
    LOGGER_ENTRY(HTTP_CREATED_PROTOCOL, DEBUG, TCP_SERVER, "FD %i: HTTP Protocol created with request served") \
    LOGGER_ENTRY(HTTP_UNKNOWN_EXTENSION, DEBUG, TCP_SERVER, "Unknown HTTP content type using text/plain") \
    \
    LOGGER_ENTRY(TEST_GUID_LOG, INFO, TEST, "IOT Error '%vg' caps '%vG'") \
    LOGGER_ENTRY(TEST_FLOAT_LOGS, INFO, TEST, "Test float %%%hf, double %f") \
    LOGGER_ENTRY(TEST_INTEGER_LOGS, INFO, TEST, "Test %%, Integer %i, long %li, long long %lli, Short %hi, Short Short %hhi, Unsigned %u, long %lu, long long %llu, Short %hu, Short Short %hhu") \
    LOGGER_ENTRY(TEST_IPV6ADDR_LOGS, INFO, TEST, "Test char %c, ipv6_socket_addr_t %vn caps: %vN; ipv6_addr_t %vi caps: %vI ipv6_port_t %vp") \
    \
    LOGGER_ENTRY(WEB_SERVER_NO_EXTENSION, DEBUG, SYSTEM, "Web Server, file without extension is not supported, ignoring") \
    LOGGER_ENTRY(WEB_SERVER_UNSUPPORTED_EXTENSION, DEBUG, SYSTEM, "Web Server, unsupported file extension, ignoring") \
    \
    LOGGER_ENTRY(MAX_LOG, VERBOSE, TEST, "Max log no entry must be made beyond this") \
    LIST_DEFINITION_END

enum class logger_level_t : uint8_t {
#define LOGGER_LEVEL_ENTRY(x) x,
    LOGGER_LEVEL_LIST
#undef LOGGER_LEVEL_ENTRY
};

constexpr const char *logger_level_string[] = {
#define LOGGER_LEVEL_ENTRY(x) {#x},
    LOGGER_LEVEL_LIST
#undef LOGGER_LEVEL_ENTRY
};

inline logger_level_t to_logger_level(const std::string level_name) {
    std::unordered_map<std::string, logger_level_t> logger_level_enum = {
#define LOGGER_LEVEL_ENTRY(x) {#x, logger_level_t::x},
        LOGGER_LEVEL_LIST
#undef LOGGER_LEVEL_ENTRY
    };

    auto level_itr = logger_level_enum.find(level_name);
    if (level_itr == logger_level_enum.end()) {
        return static_cast<logger_level_t>(0xff);
    }

    return level_itr->second;
}

class logger_level_operators {
private:
    const logger_level_t value;

public:
    constexpr logger_level_operators(const logger_level_t value) : value(value) {}

    constexpr operator const char * () const {
        switch(value) {
#define LOGGER_LEVEL_ENTRY(x) case logger_level_t::x: return #x;
    LOGGER_LEVEL_LIST
#undef LOGGER_LEVEL_ENTRY            
        }
    }
};

template <bool null_terminated = true>
constexpr size_t to_string(const logger_level_t &val, char *dest) {
    switch (val) {
    default: // This will avoid error, such condition will never reach
        assert(true);
#define LOGGER_LEVEL_ENTRY(x) \
    case logger_level_t::x: { \
        constexpr size_t desc_size = sizeof(#x) \
                - (null_terminated ? 0: 1); \
        constexpr const char desc[] = #x; \
        std::copy(desc, desc + desc_size, dest); \
        return desc_size; }
        LOGGER_LEVEL_LIST
#undef LOGGER_LEVEL_ENTRY
    }
}

enum class module_t {
#define LOGGER_MODULE_ENTRY(x) x,
    LOGGER_MODULE_LIST
#undef LOGGER_MODULE_ENTRY
};

constexpr const char *module_t_string[] = {
#define LOGGER_MODULE_ENTRY(x) {#x},
    LOGGER_MODULE_LIST
#undef LOGGER_MODULE_ENTRY
};

inline module_t to_module_t(const std::string module_name) {
    std::unordered_map<std::string, module_t> module_enum = {
#define LOGGER_MODULE_ENTRY(x) {#x, module_t::x},
        LOGGER_MODULE_LIST
#undef LOGGER_MODULE_ENTRY
    };

    auto module_itr = module_enum.find(module_name);
    if (module_itr == module_enum.end()) {
        return module_t::UNKNOWN;
    }

    return module_itr->second;
}

template <bool null_terminated = true>
constexpr size_t to_string(const module_t &val, char *dest) {
    switch (val) {
    default: // This will avoid error, such condition will never reach
        assert(true);
#define LOGGER_MODULE_ENTRY(x) \
    case module_t::x: { \
        constexpr size_t desc_size = sizeof(#x) \
                - (null_terminated ? 0: 1); \
        constexpr const char desc[] = #x; \
        std::copy(desc, desc + desc_size, dest); \
        return desc_size; }
        LOGGER_MODULE_LIST
#undef LOGGER_MODULE_ENTRY
    }
}

} // namespace MMS