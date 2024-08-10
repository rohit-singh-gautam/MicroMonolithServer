/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <sys/epoll.h>
#include <thread>
#include <rohit/base/error.h>
#include <rohit/base/types.h>
#include <rohit/log/log.h>
#include <rohit/lockfree/fixedqueue.h>

namespace rohit::event {
using namespace std::chrono_literals;

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
class listner_t {
    int epollfd;
    bool IsTerminated { false };

public:
    listner_t() : epollfd(epoll_create1(0)) {
        if (epollfd == -1) {
            log<log_t::EVENT_DIST_CREATE_FAILED>(errno);
            throw exception_t(err_t::EVENT_DIST_CREATE_FAILED);
        } else {
            log<log_t::EVENT_DIST_CREATE_SUCCESS>();
        }
    }

    err_t add(const int fd, auto CustomData) const {
        epoll_event epoll_data { EPOLLIN | EPOLLONESHOT | EPOLLRDHUP, { reinterpret_cast<void *>(CustomData) } };
        if constexpr (config::debug) {
            if (fd == 0) {
                throw exception_t(err_t::EVENT_CREATE_FAILED_ZERO);
            }
        }

        auto ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &epoll_data);

        if (ret == -1) {
            log<log_t::EVENT_CREATE_FAILED>(fd, errno);
            return err_t::EVENT_CREATE_FAILED;
        } else {
            log<log_t::EVENT_CREATE_SUCCESS>(fd);
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
            log<log_t::EVENT_ENABLE_FAILED>(fd, errno);
            return err_t::EVENT_ENABLE_FAILED;
        } else {
            log<log_t::EVENT_ENABLE_SUCCESS>(fd);
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
            log<log_t::EVENT_REMOVE_FAILED>(fd, errno);
            return err_t::EVENT_REMOVE_FAILED;
        } else {
            log<log_t::EVENT_REMOVE_SUCCESS>(fd);
            return err_t::SUCCESS;
        }
    }

    void Close() {
        IsTerminated = true;
    }

    void loop() {
        epoll_event events[rohit::config::eventlistner::maxeventcount];
        while(true) {
            auto ret = epoll_wait(epollfd, events, rohit::config::eventlistner::maxeventcount, -1);

            if (ret == -1) {
                if (errno == EINTR || errno == EINVAL) {
                    if (IsTerminated) {
                        pthread_exit(nullptr);
                    }
                }

                log<log_t::EVENT_DIST_LOOP_WAIT_INTERRUPTED>(errno);
                std::this_thread::sleep_for(1s);
                // Check again if terminated
                if (IsTerminated) {
                    pthread_exit(nullptr);
                }
                continue;
            }

            for(decltype(ret) index = 0; index < ret; ++index) {
                epoll_event &event = events[index];
                auto processor = reinterpret_cast<event::processor_t *>(event.data.ptr);

                log<log_t::EVENT_DIST_EVENT_RECEIVED>(processor->GetFD(), event.events);
                if ((event.events & EPOLLRDHUP)) {
                    delete processor;
                } else {
                    err_t ret { err_t::SUCCESS };
                    if ((event.events & (EPOLLIN | EPOLLHUP | EPOLLERR))) {
                        // EPOLLHUP | EPOLLERR
                        // recv() will return 0 for EPOLLHUP and -1 for EPOLLERR
                        // recv() 0 means end of file.
                        ret = processor->ProcessRead();
                        if (ret == err_t::SOCKET_RETRY) {
                            event.events |= EPOLLOUT;
                        }
                    }
                    if ((event.events & EPOLLOUT)) {
                        ret = processor->ProcessWrite();
                    }

                    switch(ret) {
                        case err_t::SUCCESS:
                            enable(processor, false);
                            break;
                        
                        // SOCKET_RETRY will only happen for Write
                        // read converts it to SUCCESS
                        case err_t::SOCKET_RETRY:
                            enable(processor, true);
                            break;

                        // case err_t::BAD_FILE_DESCRIPTOR:
                        default:
                            delete processor;
                            break;

                    }
                }
            }
        }
    }
};

} // namespace rohit::event