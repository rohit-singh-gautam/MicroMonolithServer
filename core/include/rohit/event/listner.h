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

class processor_t {
public:
    using ProcessorFunction = void (*)(processor_t *);

protected:
    friend class executor_t;

    volatile bool to_read { false };
    volatile bool to_write { false };
    volatile bool to_terminate { false };

public:
    virtual ~processor_t() = default;
    // Using default constructor

    constexpr void SetToRead() { to_read = true; }
    constexpr void SetToWrite() { to_write = true; }
    constexpr void SetToTerminate() { to_terminate = true; }

    virtual int GetFD() const = 0;
    
    virtual err_t ProcessRead() = 0;

    /*! Write is optional in some cases */
    virtual err_t ProcessWrite() { return err_t::SUCCESS; }
};

class listner_t;

/*! Event queue is designed for write from one thread and read for another thread*/
class executor_t {
private:
    lockfree::fixedqueue_t<processor_t *> StorageQueue;
    std::atomic<size_t> &allqueuesize;

    bool terminate { false };

    listner_t *listner;

public:
    constexpr executor_t(listner_t *listner, auto executor_queue_size, std::atomic<size_t> &allqueuesize) 
        : StorageQueue { executor_queue_size }, allqueuesize { allqueuesize }, listner{ listner } {}
    constexpr  ~executor_t() { terminate = true; }
    executor_t(const executor_t &) = default;
    executor_t &operator=(const executor_t &) = default;

    constexpr inline bool full() {
        return StorageQueue.full();
    }

    constexpr inline bool empty() {
        return StorageQueue.empty();
    }

    constexpr const auto size() {
        return StorageQueue.size();
    }

    constexpr inline bool AddToQueue(processor_t *Data) {
        return StorageQueue.push_back(Data);
    }

    constexpr inline void AddToQueueUnsafe(processor_t *Data) {
        StorageQueue.push_back_unsafe(Data);
        ++allqueuesize;
    }

    constexpr bool IsValid() { return !terminate; }

    constexpr void Close() { terminate = true; }

    constexpr void ProcessAll();

    void loop() {
        while(!terminate) {
            ProcessAll();
            // Unless it is a realtime listner sleeping for 10ms is OK if we find queue is empty.
            // this will drastically reduce CPU usage from 100% to less than 0.1%
            std::this_thread::sleep_for(10ms);
        }
    }
};

/*! Event Executor will try to add entry too event queue if event queue is full it will be added to Overflow queue.
 *  To add to queue it will pick up smallest from next 4. In this way eventually smallest queue will be filled.
 *  Instead of adding one at a time event executor can add multiple entries based on it parameter.
 *  Entire multiplexer_t class is designed to be in single thread.
*/
class multiplexer_t {
    const size_t ThreadCount;
    std::unique_ptr<std::unique_ptr<executor_t>[]> executors { };
    std::queue<processor_t *> OverflowQueue { };
    std::mutex AddToQueueMutex { };

    std::atomic<size_t> executorqueuesize { 0 };
    
    static constexpr size_t max_process_to_search { 4 };
    const size_t process_to_search;
    size_t NextProcessor { 0 };

    constexpr auto GetLeastBusyProcessorHelper() {
        size_t minindex { NextProcessor };
        auto mincount = executors[NextProcessor]->size();
        for(size_t index { 1 }; index < process_to_search; ++index) {
            const auto currentindex = (NextProcessor + index) % ThreadCount;
            const auto currentcount = executors[currentindex]->size();
            if (currentcount < mincount) {
                minindex = currentindex;
                mincount = currentcount;
            }
        }

        NextProcessor = (NextProcessor + process_to_search) % ThreadCount;
        return std::make_pair(minindex,  mincount);
    }

    constexpr size_t GetLeastBusyProcessor() {
        const auto averagesize = executorqueuesize/ThreadCount;
        const auto maxattempt = ThreadCount/max_process_to_search;
        for(size_t index { 0 }; index < maxattempt - 1; ++index) {
            auto [minindex, mincount] = GetLeastBusyProcessorHelper();
            if (mincount <= averagesize) return minindex;
        }
        auto [minindex, mincount] = GetLeastBusyProcessorHelper();
        return minindex;
    }


    std::vector<std::jthread> allthread { };

    listner_t *listner;

public:
    /*! If TheadCount is zero it will be auto calculated using CPU count*/
    multiplexer_t(listner_t *listner, const size_t ThreadCount, const size_t executor_queue_size)
        : ThreadCount { ThreadCount ? ThreadCount : sysconf(_SC_NPROCESSORS_ONLN) }, executors { new std::unique_ptr<executor_t>[ThreadCount] },
          process_to_search { std::min(max_process_to_search, ThreadCount)}, listner { listner }
    {
        for(size_t index { 0 }; index < ThreadCount; ++index) {
            executors[index] = std::make_unique<executor_t>(listner, executor_queue_size, executorqueuesize);
            std::jthread currentthread(&executor_t::loop, executors[index].get());
            allthread.emplace_back(std::move(currentthread));
        }
    }

    multiplexer_t(const multiplexer_t &) = default;
    multiplexer_t &operator=(const multiplexer_t &) = default;

    constexpr inline void AddToQueue(processor_t *Processor) {
        std::lock_guard add_lock { AddToQueueMutex }; 

        const auto NextProcessor = GetLeastBusyProcessor();
        auto &CurrentProcessor = executors[NextProcessor];
        auto IsAdded = CurrentProcessor->AddToQueue(Processor);

        // If process is already executing putting it in overflow queue gives it a second chance
        if (!IsAdded) {
            OverflowQueue.push(Processor);
        }
    }

    constexpr inline void Cleanup() {
         while(!OverflowQueue.empty()) {
            auto processor = OverflowQueue.front();
            const auto NextProcessor = GetLeastBusyProcessor();
            auto &CurrentProcessorQueue = executors[NextProcessor];
            if (CurrentProcessorQueue->full()) break;
            CurrentProcessorQueue->AddToQueueUnsafe(processor);
            OverflowQueue.pop();
        }
    }

};

/*! 
 * This class will poll for filediscriptor and pull all the data in multiplexer_t.
 * multiplexer_t implementation can be multi threaded or single threaded.
 */
class listner_t {
    multiplexer_t multiplexer;

    int epollfd;
    bool IsTerminated { false };

public:
    listner_t(const size_t ThreadCount, const size_t executor_queue_size = 192UL) : multiplexer { this, ThreadCount, executor_queue_size }, epollfd(epoll_create1(0)) {
        if (epollfd == -1) {
            log<log_t::EVENT_DIST_CREATE_FAILED>(errno);
            throw exception_t(err_t::EVENT_DIST_CREATE_FAILED);
        } else {
            log<log_t::EVENT_DIST_CREATE_SUCCESS>();
        }
    }

    err_t add(const int fd, auto CustomData, bool disabled = false) const {
        epoll_event epoll_data { EPOLLONESHOT | EPOLLRDHUP, { reinterpret_cast<void *>(CustomData) } };
        if (!disabled) epoll_data.events |= EPOLLIN | EPOLLONESHOT | EPOLLRDHUP;
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

    err_t add(processor_t &processor, bool disabled = false) {
        return add(processor.GetFD(), &processor, disabled);
    }

    err_t add(processor_t *processor, bool disabled = false) {
        return add(processor->GetFD(), processor, disabled);
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

    constexpr inline void AddToQueue(processor_t *processor) {
        log<log_t::EVENT_LISTNER_NEW_ADDED>(processor->GetFD());
        multiplexer.AddToQueue(processor);
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
                    // First remove from the listner then added to terminate
                    remove(processor->GetFD());

                    processor->SetToTerminate();
                } else {
                    if ((event.events & (EPOLLIN | EPOLLHUP | EPOLLERR))) {
                        // EPOLLHUP | EPOLLERR
                        // recv() will return 0 for EPOLLHUP and -1 for EPOLLERR
                        // recv() 0 means end of file.
                        processor->SetToRead();
                    }
                    if ((event.events & EPOLLOUT)) {
                        processor->SetToWrite();
                    }
                }
                multiplexer.AddToQueue(processor);
            }
            multiplexer.Cleanup();
        }
    }
};

constexpr void executor_t::ProcessAll() {
    while (StorageQueue) {
        auto currentprocessor = StorageQueue.get_pop_front();
        --allqueuesize;
        if (currentprocessor->to_terminate) {
            delete currentprocessor;
        } else {
            // TODO: Return value of ProcessRead and ProcessWrite socket closed
            // currentprocess must object must be deleted
            err_t ret { err_t::SUCCESS };
            if (currentprocessor->to_read) {
                currentprocessor->to_read = false;
                ret = currentprocessor->ProcessRead();
            }

            if (ret == err_t::BAD_FILE_DESCRIPTOR) {
                delete currentprocessor;
                return;
            }

            if (currentprocessor->to_write) {
                currentprocessor->to_write = false;
                ret = currentprocessor->ProcessWrite();
            }
            switch(ret) {
                case err_t::SUCCESS:
                    listner->enable(currentprocessor, false);
                    break;
                
                case err_t::SOCKET_RETRY:
                    listner->enable(currentprocessor, true);
                    break;

                case err_t::BAD_FILE_DESCRIPTOR:
                default:
                    delete currentprocessor;
                    break;

            }
            
        }
    }
}

} // namespace rohit::event