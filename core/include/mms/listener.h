/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <sys/epoll.h>
#include <thread>
#include <mms/base/error.h>
#include <mms/base/types.h>
#include <mms/log/log.h>
#include <mms/lockfree/fixedqueue.h>
#include <unordered_set>
#include <typeinfo>

namespace MMS::listener {
using namespace std::chrono_literals;

struct write_entry_const {
    const uint8_t *buffer;
    size_t size;
    size_t offset;
    template <typename buffertype>
    constexpr write_entry_const(buffertype buffer, size_t bytesize, size_t byteoffset = 0) : buffer { reinterpret_cast<const uint8_t *>(buffer) }, size { bytesize }, offset { byteoffset } { }
};

struct write_entry {
private:
    uint8_t *buffer;
    size_t size;
    size_t offset;
public:
    template <typename buffertype>
    constexpr write_entry(buffertype buffer, size_t bytesize, size_t byteoffset = 0) : buffer { reinterpret_cast<uint8_t *>(buffer) }, size { bytesize }, offset { byteoffset } { }
    ~write_entry() {
        // No check is require before free as
        // Empty write_entry is not allowed.
        free(buffer);
    }

    constexpr write_entry(const write_entry &) = delete;
    constexpr write_entry &operator=(const write_entry &) = delete;

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

    constexpr auto RemainingBuffer() const {
        return size - offset;
    }

    constexpr auto Completed() const { return offset >= size; }
    constexpr auto Pending() const { return offset < size; }
};


namespace typecheck {
template <typename T>
concept write_entry_const = std::is_same_v<T, MMS::listener::write_entry_const>;
} // namespace typecheck

class writer_t {
protected:
    // Buffer will be deleted once it is used.
    // All buffer must be created using malloc
    virtual void WriteNoCopy(uint8_t*, size_t, size_t) { };
    
    inline void WriteWithCopy(const uint8_t* buffer, size_t, size_t bytesize) {
        auto newbuffer = reinterpret_cast<uint8_t *>(malloc(bytesize));
        std::copy(buffer, buffer + bytesize, newbuffer);
        WriteNoCopy(newbuffer, bytesize, 0);
    }

public:
    virtual ~writer_t() = default;

    template <bool CopyBuffer = true, typename buffertype>
    inline void Write(buffertype buffer, size_t bytesize, size_t byteoffset = 0) {
        if constexpr (CopyBuffer) WriteWithCopy(buffer, bytesize, byteoffset);
        else WriteNoCopy(buffer, bytesize, byteoffset);
    }

    inline void Write(const std::string &buffer) {
        WriteWithCopy(reinterpret_cast<const uint8_t*>(buffer.c_str()), buffer.size(), 0);
    }

    inline void Write(const std::string_view &buffer) {
        WriteWithCopy(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size(), 0);
    }


    template <typecheck::write_entry_const... buffertype>
    inline void Write(const buffertype&... buffer) {
        auto buffersize = ((buffer.size - buffer.offset) + ...);
        auto newbuffer = reinterpret_cast<uint8_t *>(malloc(buffersize));
        auto outputitr = newbuffer;

        ((outputitr = std::copy(buffer.buffer + buffer.offset, buffer.buffer + buffer.size, outputitr)), ...);
        WriteNoCopy(newbuffer, buffersize, 0);
    }
};

class listener_t;

class processor_t : public writer_t {
    int fd;
protected:
    friend class listener_t;

    processor_t(int fd) : fd { fd } { }
    virtual ~processor_t() {
        if (fd) {
            close(fd);
            fd = 0;
        }
    }
public:
    int GetFD() const { return fd; }
    
    /*! To close this processor ProcessRead must return err_t::INITIATE_CLOSE */
    virtual err_t ProcessRead() = 0;

    /*! Write is optional in some cases */
    virtual err_t ProcessWrite() { return err_t::SUCCESS; }

    void Close() {
        ::close(fd);
        fd = 0;
    }
};

class terminate_t : public processor_t {
    listener_t &listener;
public:
    terminate_t(listener_t &listener);
    err_t ProcessRead() override;
}; // terminate_t

class thread_stopper_t : public processor_t {
    volatile bool running;
public:
    thread_stopper_t();
    err_t ProcessRead() override;

    auto GetRunning() const { return running; }
    void SetRunning() { running = true;}
};

/*! Only one instannce of listener can be created.
 * Any attempt to create more than one instance will throw listener_already_created_t exception.
*/
class listener_t {
public:
    static constexpr size_t max_event_epoll_return_default { 8 };
private:
    friend class terminate_t;
    size_t threadcount;

    int epollfd { };

    // This is number of event that can be returned from epoll in one wait
    // For single threaded this can be very high.
    const size_t max_event_epoll_return;
    bool IsTerminated { false };

    std::vector<std::jthread> threadlist { };

    std::jthread log_thread { };
    void log_thread_function();
    void init_log_thread(const std::filesystem::path &filename);

    terminate_t terminatehandler;

    static bool created;

#ifdef MMS_LISTENER_CLOSE_AT_EXIT
    std::unordered_set<processor_t *> active_processors { };
#endif

    void Delete(processor_t *processor) {
        remove(processor);
        delete processor;
    }

public:
    listener_t(const size_t threadcount, const std::filesystem::path &filename, const size_t max_event_epoll_return = max_event_epoll_return_default);
    ~listener_t();

    err_t add(processor_t *processor) {
        const auto fd = processor->GetFD();
        epoll_event epoll_data { EPOLLIN | EPOLLONESHOT | EPOLLRDHUP, { reinterpret_cast<void *>(processor) } };
        auto ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &epoll_data);

        if (ret == -1) {
            log<log_t::LISTNER_EVENT_ADD_FAILED>(fd, errno);
            return err_t::LISTNER_EVENT_ADD_FAILED;
        } else {
#ifdef MMS_LISTENER_CLOSE_AT_EXIT
            active_processors.insert(processor);
#endif
            log<log_t::LISTNER_EVENT_ADD_SUCCESS>(fd);
            return err_t::SUCCESS;
        }
    }

    err_t enable(processor_t *processor, bool enablewrite) const {
        const auto fd = processor->GetFD();
        epoll_event epoll_data { EPOLLIN | EPOLLONESHOT | EPOLLRDHUP, { reinterpret_cast<void *>(processor) } };
        if (enablewrite) epoll_data.events |= EPOLLOUT;
        auto ret = epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &epoll_data);

        if (ret == -1) {
            log<log_t::LISTNER_EVENT_ENABLE_FAILED>(fd, errno);
            return err_t::LISTNER_EVENT_ENABLE_FAILED;
        } else {
            log<log_t::LISTNER_EVENT_ENABLE_SUCCESS>(fd);
            return err_t::SUCCESS;
        }
    }

    err_t remove(processor_t *processor) {
        const auto fd = processor->GetFD();
        auto ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
#ifdef MMS_LISTENER_CLOSE_AT_EXIT
        active_processors.erase(processor);
#endif

        if (ret == -1) {
            log<log_t::LISTNER_EVENT_REMOVE_FAILED>(fd, errno);
            return err_t::LISTNER_EVENT_REMOVE_FAILED;
        } else {
            log<log_t::LISTNER_EVENT_REMOVE_SUCCESS>(fd);
            return err_t::SUCCESS;
        }
    }

    auto GetThreadCount() const { return threadcount; }

    void close();

    void loop();

    void multithread_loop() {

        log<log_t::LISTENER_CREATING_THREAD>(threadcount);
        for(size_t index { 0 }; index < threadcount; ++index) {
            threadlist.emplace_back(&listener_t::loop, this);
        }
    }

    void wait() {
        for(auto &thread: threadlist) {
            thread.join();
            log<log_t::LISTENER_EXIT_THREAD_JOIN_SUCCESS>();
        }
    }
};

} // namespace MMS::listener