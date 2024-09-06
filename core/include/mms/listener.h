/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/base/stream.h>
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
class writer_t {
public:
    virtual ~writer_t() = default;

    // Buffer will be deleted once it is used.
    // All buffer must be created using malloc
    virtual void WriteNoCopy(Stream &&) { };
    
    inline void WriteWithCopy(const ConstStream &stream) {
        auto newbuffer = reinterpret_cast<uint8_t *>(malloc(stream.remaining_buffer()));
        std::copy(stream.curr(), stream.end(), newbuffer);
        WriteNoCopy(FullStream {newbuffer, stream.remaining_buffer()});
    }

    inline void WriteWithCopy(const Stream &stream) {
        auto newbuffer = reinterpret_cast<uint8_t *>(malloc(stream.remaining_buffer()));
        std::copy(stream.curr(), stream.end(), newbuffer);
        WriteNoCopy(FullStream {newbuffer, stream.remaining_buffer()});
    }

    inline void Write(const std::string &buffer) {
        auto newbuffer = reinterpret_cast<char *>(malloc(buffer.size()));
        std::copy(std::begin(buffer), std::end(buffer), newbuffer);
        WriteNoCopy(FullStream {newbuffer, buffer.size()});
    }

    inline void Write(const std::string_view &buffer) {
        auto newbuffer = reinterpret_cast<char *>(malloc(buffer.size()));
        std::copy(std::begin(buffer), std::end(buffer), newbuffer);
        WriteNoCopy(FullStream {newbuffer, buffer.size()});
    }


    template <typecheck::ConstStream... buffertype>
    inline void Write(const buffertype&... buffer) {
        auto buffersize = ((buffer.remaining_buffer()) + ...);
        auto newbuffer = reinterpret_cast<uint8_t *>(malloc(buffersize));
        auto outputitr = newbuffer;

        ((outputitr = std::copy(buffer.curr(), buffer.end(), outputitr)), ...);
        WriteNoCopy(FullStream {newbuffer, buffersize});
    }
};

class listener_t;

class processor_t : public writer_t {
    int fd;
protected:
    friend class listener_t;

    processor_t(int fd) : fd { fd } { }

public:
    virtual ~processor_t() {
        if (fd) {
            close(fd);
            fd = 0;
        }
    }
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
    listener_t(const std::filesystem::path &filename, const size_t max_event_epoll_return = max_event_epoll_return_default);
    ~listener_t();

    err_t add(processor_t *processor) {
        const auto fd = processor->GetFD();
        epoll_event epoll_data { EPOLLIN | EPOLLONESHOT | EPOLLRDHUP, { reinterpret_cast<void *>(processor) } };
        auto ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &epoll_data);

        if (ret == -1) {
            log<log_t::LISTNER_EVENT_ADD_FAILED>(fd, errno);
            return err_t::LISTNER_EVENT_ADD_FAILED;
        } else {
#ifdef MMS_LISTENER_CLOSE_AT_EXIT // Enabling MMS_LISTENER_CLOSE_AT_EXIT will cause double free in repository for server types
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

    size_t SetThreadCount(size_t threadcount);

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

    void ForceTerminateLogThread() { IsTerminated = false; }
};

} // namespace MMS::listener