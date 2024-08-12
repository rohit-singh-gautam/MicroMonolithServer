/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/event/listener.h>
#include <mms/base/types.h>
#include <mms/net/socket.h>

namespace MMS::event {

struct buffer_t {
public:
    static constexpr size_t initial_size { 256 };

private:
    void *buffer;
    size_t size;

public:
    buffer_t(const size_t size = 512UL) : buffer { malloc(initial_size)}, size { size } { }
    buffer_t(const buffer_t &) = delete;
    buffer_t &operator=(const buffer_t &) = delete;

    ~buffer_t() {
        if (buffer) free(buffer);
    }

    void increase_buffer(const size_t newsize) {
        if (newsize > size) {
            buffer = realloc(buffer, newsize);
            size = newsize;
        }
    }

    template <typename type>
    auto GetBuffer() { return reinterpret_cast<type>(buffer); }
    auto GetBuffer(const size_t offset) {
        auto bufferoffset = reinterpret_cast<uint8_t *>(buffer) + offset;
        auto newsize = size - offset;
        return std::make_pair(reinterpret_cast<void *>(bufferoffset), newsize);
    }


};

struct write_entry {
private:
    uint8_t *buffer { nullptr };
    size_t offset { 0 };
    size_t size { 0 };
public:
    constexpr write_entry() { }
    template <typename buffertype>
    constexpr write_entry(buffertype buffer, size_t bytesize, size_t byteoffset = 0) : buffer { reinterpret_cast<uint8_t *>(buffer) }, offset { byteoffset }, size { bytesize } { }


    constexpr write_entry(const write_entry &) = default;
    constexpr write_entry &operator=(const write_entry &) = default;

    template <typename type>
    constexpr auto GetBufferBase() { return reinterpret_cast<type>(buffer); }

    template <typename type>
    constexpr auto GetBuffer() { 
        auto newbuffer = buffer + offset;
        const auto newsize = size - offset;
        return std::make_pair(reinterpret_cast<type>(newbuffer), newsize);
    }

    constexpr auto AddOffset(const size_t offset) {
        this->offset += offset;
    }

    constexpr auto Completed() const { return offset >= size; }
    constexpr auto Pending() const { return offset < size; }

    constexpr void Delete() {
        free(buffer);
        size = offset = 0;
    }
};

namespace tcp {
class connection_t;
}

class protocol_implementation_t;

class protocol_implementation_t {
public:
    virtual ~protocol_implementation_t() = default;

    virtual void ProcessRead(const uint8_t *buffer, const size_t size, writer_t &writer) = 0;
};

class protocol_implementation_creator_t {
public:
    virtual ~protocol_implementation_creator_t() = default;

    virtual protocol_implementation_t *create_protocol_implementation() = 0;
};

namespace tcp {
class connection_t : public processor_t {
    socket_t connection_socket;
    protocol_implementation_t *const protocol_implementation;
    std::queue<write_entry> pending_wirte { };

    static thread_local buffer_t tempbuffer;

public:
    connection_t(socket_t &&connection_socket, protocol_implementation_t * const protocol_implementation)
        : connection_socket { std::move(connection_socket) }, protocol_implementation { protocol_implementation } { }
    connection_t(const connection_t&) = delete;
    connection_t& operator=(const connection_t&) = delete;


    err_t ProcessRead() override {
        size_t offset { 0 };
        while(true) {
            size_t readlen  { 0 };
            auto [buffer, buffer_size] = tempbuffer.GetBuffer(offset);
            auto ret = connection_socket.read(buffer, buffer_size, readlen);
            offset += readlen;
            if (ret == err_t::SUCCESS) continue;
            if (ret == err_t::SOCKET_RETRY) break;
            if (ret == err_t::CONNECTION_REFUSED || ret == err_t::RECEIVE_FAILURE) {
                log<log_t::TCP_SERVER_PEER_READ_FAILED>(connection_socket.GetFD(), errno);
                // Close will take care of termination
                connection_socket.close();
                return err_t::BAD_FILE_DESCRIPTOR;
            }
            return ret;
        }
        if (offset) {
            protocol_implementation->ProcessRead(tempbuffer.GetBuffer<uint8_t *>(), offset, *this);
            log<log_t::TCP_CONNECTION_READ>(GetFD(), offset);
        }
        else log<log_t::TCP_CONNECTION_EMPTY_READ>(GetFD());
        return pending_wirte.empty() ? err_t::SUCCESS : err_t::SOCKET_RETRY;
    }

    err_t ProcessWrite() override {
        while(!pending_wirte.empty()) {
            auto currentbuffer = pending_wirte.front();
            while(!currentbuffer.Completed()) {
                size_t writtenlen { 0 };
                auto [buffer, size] = currentbuffer.GetBuffer<void *>();
                auto ret = connection_socket.write(buffer, size, writtenlen);
                currentbuffer.AddOffset(writtenlen);
                if (ret == err_t::SUCCESS) continue;
                if (ret == err_t::CONNECTION_REFUSED || ret == err_t::RECEIVE_FAILURE) {
                    // Close will take care of termination
                    log<log_t::TCP_SERVER_PEER_WRITE_FAILED>(connection_socket.GetFD(), errno);
                    connection_socket.close();
                    return err_t::BAD_FILE_DESCRIPTOR;
                }
                return ret;
            }

            if (!currentbuffer.Completed()) break;
            pending_wirte.pop();
        }
        return err_t::SUCCESS;
    }

    int GetFD() const override { 
        return connection_socket.GetFD();
    }

    auto get_peer_ipv6_addr() const {
        return connection_socket.get_peer_ipv6_addr();
    }

    // If copy buffer is false use buffer as it is
    // Buffer will be deleted once it is used.
    // All buffer must be created using malloc.
    void WriteNoCopy(uint8_t* buffer, size_t bytesize, size_t byteoffset) override {
        pending_wirte.emplace(buffer, bytesize, byteoffset);
    }

    void WriteWithCopy(const uint8_t* buffer, size_t bytesize, size_t byteoffset) override {
        auto oldbuffer = reinterpret_cast<const uint8_t *>(buffer);
        auto newbuffer = reinterpret_cast<uint8_t *>(malloc(bytesize));
        std::copy(oldbuffer, oldbuffer + bytesize, newbuffer);
        pending_wirte.emplace(newbuffer, bytesize, byteoffset );
    }
};

class server_t : public processor_t {
    server_socket_t server_socket;
    protocol_implementation_creator_t &protocol_implementation_creator;
    event::listener_t *listener;
public:
    server_t(const int port, protocol_implementation_creator_t &protocol_implementation_creator, event::listener_t *listener) : server_socket { port }, protocol_implementation_creator { protocol_implementation_creator }, listener { listener } { }
    server_t(const server_t &) = default;
    server_t &operator=(const server_t &) = default;

    err_t ProcessRead() override {
        log<log_t::TCP_SERVER_RECEIVED_EVENT>(server_socket.GetFD());
        try {
            auto connection_socket = server_socket.accept();
            auto protocol_implementation = protocol_implementation_creator.create_protocol_implementation();
            auto connection = new connection_t(std::move(connection_socket), protocol_implementation);
            listener->add(connection);
            log<log_t::TCP_SERVER_PEER_CREATED>(GetFD(), connection->GetFD(), connection->get_peer_ipv6_addr());
        } catch (const exception_t e) {
            if (e == err_t::ACCEPT_FAILURE) {
                log<log_t::TCP_SERVER_ACCEPT_FAILED>(GetFD(), errno);
            }
        }
        return err_t::SUCCESS;
    }

    void Close() {
        server_socket.close();
        // Once socket is closed listener will schedule EPOLLRDHUP
        // And remove connection from polling
    }

    int GetFD() const override { 
        return server_socket.GetFD();
    }
};

} // namespace tcp

} // namespace MMS::event