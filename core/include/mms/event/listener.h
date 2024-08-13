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

namespace MMS::event {
using namespace std::chrono_literals;

struct write_entry_const {
    const uint8_t *buffer;
    size_t offset;
    size_t size;
    template <typename buffertype>
    constexpr write_entry_const(buffertype buffer, size_t bytesize, size_t byteoffset = 0) : buffer { reinterpret_cast<const uint8_t *>(buffer) }, offset { byteoffset }, size { bytesize } { }
};

struct write_entry {
private:
    uint8_t *buffer;
    size_t offset;
    size_t size;
public:
    template <typename buffertype>
    constexpr write_entry(buffertype buffer, size_t bytesize, size_t byteoffset = 0) : buffer { reinterpret_cast<uint8_t *>(buffer) }, offset { byteoffset }, size { bytesize } { }
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
concept write_entry_const = std::is_same_v<T, MMS::event::write_entry_const>;
} // namespace typecheck

class writer_t {
protected:
    virtual void WriteNoCopy(uint8_t*, size_t, size_t) { };
    virtual void WriteWithCopy(const uint8_t*, size_t, size_t) { };

public:
    virtual ~writer_t() = default;

    template <bool CopyBuffer, typename buffertype>
    inline void Write(buffertype buffer, size_t bytesize, size_t byteoffset = 0) {
        if constexpr (CopyBuffer) WriteWithCopy(buffer, bytesize, byteoffset);
        else WriteNoCopy(buffer, bytesize, byteoffset);
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

class processor_t : public writer_t {
public:
    using ProcessorFunction = void (*)(processor_t *);

protected:
    friend class executor_t;

public:
    virtual ~processor_t() = default;

    virtual int GetFD() const = 0;
    
    virtual err_t ProcessRead() = 0;

    /*! Write is optional in some cases */
    virtual err_t ProcessWrite() { return err_t::SUCCESS; }
};

/*! 
 * This class will poll for filediscriptor and pull all the data in multiplexer_t.
 * multiplexer_t implementation can be multi threaded or single threaded.
 */
class listener_t {
public:
    static constexpr size_t max_event_epoll_return_default { 8 };
private:
    int epollfd { };

    // This is number of event that can be returned from epoll in one wait
    // For single threaded this can be very high.
    const size_t max_event_epoll_return;
    bool IsTerminated { false };

    std::vector<std::jthread> threadlist { };

    std::jthread log_thread { };
    void log_thread_function();
    void init_log_thread(const std::filesystem::path &filename);

    static bool created;

public:
    listener_t(const std::filesystem::path &filename, const size_t max_event_epoll_return = max_event_epoll_return_default);

    err_t add(const int fd, auto CustomData) const {
        epoll_event epoll_data { EPOLLIN | EPOLLONESHOT | EPOLLRDHUP, { reinterpret_cast<void *>(CustomData) } };
#ifdef DEBUG
            if (fd == 0) {
                throw exception_t(err_t::LISTENER_CREATE_FAILED_ZERO);
            }
#endif

        auto ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &epoll_data);

        if (ret == -1) {
            log<log_t::LISTNER_EVENT_ADD_FAILED>(fd, errno);
            return err_t::LISTNER_EVENT_ADD_FAILED;
        } else {
            log<log_t::LISTNER_EVENT_ADD_SUCCESS>(fd);
            return err_t::SUCCESS;
        }
    }

    err_t add(processor_t &processor) {
        return add(processor.GetFD(), &processor);
    }

    err_t add(processor_t *processor) {
        return add(processor->GetFD(), processor);
    }

    err_t enable(const int fd, auto CustomData, bool enablewrite) const {
        epoll_event epoll_data { EPOLLIN | EPOLLONESHOT | EPOLLRDHUP, { reinterpret_cast<void *>(CustomData) } };
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

    err_t enable(processor_t &processor, bool enablewrite) {
        return enable(processor.GetFD(), &processor, enablewrite);
    }

    err_t enable(processor_t *processor, bool enablewrite) {
        return enable(processor->GetFD(), processor, enablewrite);
    }

    err_t remove(const int fd) {
        auto ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
        if (ret == -1) {
            log<log_t::LISTNER_EVENT_REMOVE_FAILED>(fd, errno);
            return err_t::LISTNER_EVENT_REMOVE_FAILED;
        } else {
            log<log_t::LISTNER_EVENT_REMOVE_SUCCESS>(fd);
            return err_t::SUCCESS;
        }
    }

    void Close() {
        IsTerminated = true;
    }

    void loop();

    void multithread_loop(size_t threadcount) {
        size_t cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
        if (!threadcount) threadcount = cpu_count;
        if (threadcount > cpu_count) {
            log<log_t::LISTENER_TOO_MANY_THREAD>();
        }
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

} // namespace MMS::event