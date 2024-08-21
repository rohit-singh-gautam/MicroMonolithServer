/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/listener.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/eventfd.h>


namespace MMS::listener {

static int CreateSignalFD() {
    sigset_t sigmaskignore { };
    sigemptyset(&sigmaskignore);
    sigaddset(&sigmaskignore, SIGTERM);
    sigaddset(&sigmaskignore, SIGINT);
    int ret = sigprocmask(SIG_BLOCK, &sigmaskignore, 0);
    if (ret == -1) {
        log<log_t::SIGNAL_POLLING_MASK_FAILED>(errno);
        throw signal_polling_failed_t();
    }

    sigset_t sigmask { };
    sigaddset(&sigmask, SIGTERM);

    // This is a blocking fd
    auto signal_fd = signalfd(-1, &sigmask, SFD_CLOEXEC);
    if (signal_fd == -1) {
        log<log_t::SIGNAL_FD_FAILED>(errno);
        throw signal_polling_failed_t();
    }
    return signal_fd;
}

terminate_t::terminate_t(listener_t &listener) : processor_t { CreateSignalFD() }, listener { listener } {

}

err_t terminate_t::ProcessRead() {
    listener.remove(this);

    signalfd_siginfo siginfo { };
    auto ret = read(GetFD(), &siginfo, sizeof(siginfo));
    if (ret == -1) {
        log<log_t::SIGNAL_READ_FAILED>(errno);
        throw signal_read_failed_t();
    }
    
    thread_stopper_t stopper { };

    auto stopthread { listener.GetThreadCount() - 1 };
    listener.add(&stopper);
    for(; stopthread; --stopthread) {
        while(stopper.GetRunning()) {
            // This will reduce the CPU usage to almost zero
            std::this_thread::sleep_for(10ms);
        }
        stopper.SetRunning();
        listener.enable(&stopper, false);
    }

    listener.IsTerminated = true;
    listener.remove(&stopper);
    close(listener.epollfd);
    
    listener.close();
    MMS::logger::all.flush();
    throw listener_terminate_thread_t();
}

thread_stopper_t::thread_stopper_t() : processor_t { eventfd(1, EFD_NONBLOCK | EFD_CLOEXEC)  }, running { true } {}

err_t thread_stopper_t::ProcessRead() {
    running = false;
    throw listener_terminate_thread_t { };
}

void listener_t::close() {
#ifdef MMS_LISTENER_CLOSE_AT_EXIT
    for (auto processor: active_processors) {
        // We must not call listener remove here as it will modify active_processors
        // While iterating through active_processors, it must not be modified.
        delete processor;
    }
    active_processors.clear();
#endif
}

bool listener_t::created { false };

listener_t::listener_t(const size_t threadcount, const std::filesystem::path &filename, const size_t max_event_epoll_return) 
    :  threadcount { threadcount }, max_event_epoll_return { max_event_epoll_return }, terminatehandler { *this }
{
    size_t cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (!threadcount) this->threadcount = cpu_count;
    if (this->threadcount > cpu_count) {
        log<log_t::LISTENER_TOO_MANY_THREAD>();
    }

    if (created) {
        log<log_t::LISTENER_ALREADY_CREATED_FAILED>();
        throw listener_already_created_t { };
    }
    created = true;

    init_log_thread(filename);

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        log<log_t::LISTENER_CREATE_FAILED>(errno);
        throw listener_create_failed_t { };
    }

    add(&terminatehandler);

    log<log_t::LISTENER_CREATE_SUCCESS>();
}

listener_t::~listener_t() {
    MMS::logger::all.flush();
}

void listener_t::init_log_thread(const std::filesystem::path &filename) {
    if (!std::filesystem::exists(filename)) {
        auto parent { filename.parent_path() };
        std::filesystem::create_directories(parent);
    }
    int log_filedescriptor = open(filename.c_str(), O_RDWR | O_APPEND | O_CREAT, O_SYNC | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if ( log_filedescriptor < 0 ) {
        std::cerr << "Failed to open file " << filename.c_str() << ", error " << errno << ", " << strerror(errno) << std::endl;
    }

    MMS::logger::all.set_fd(log_filedescriptor);

    log_thread = std::jthread { &listener_t::log_thread_function, this };
}

void listener_t::log_thread_function() {
    constexpr auto wait_time = std::chrono::milliseconds(200);
    while(!IsTerminated) {
        std::this_thread::sleep_for(wait_time);
        MMS::logger::all.flush();
    }

    MMS::logger::all.flush();
}

void listener_t::loop() {
    log<log_t::LISTENER_LOOP_CREATED>();
    auto events = std::make_unique<epoll_event[]>(max_event_epoll_return);
    try {
        for(;;) {
            auto ret = epoll_wait(epollfd, events.get(), max_event_epoll_return, -1);

            if (ret == -1) {
                log<log_t::LISTENER_LOOP_WAIT_INTERRUPTED>(errno);
                std::this_thread::sleep_for(1s);
                continue;
            }

            for(decltype(ret) index = 0; index < ret; ++index) {
                epoll_event &event = events[index];
                auto processor = reinterpret_cast<listener::processor_t *>(event.data.ptr);

                log<log_t::LISTENER_EVENT_RECEIVED>(processor->GetFD(), event.events);
                if ((event.events & EPOLLRDHUP)) {
                    log<log_t::TCP_SERVER_PEER_CONNECTION_CLOSED>(processor->GetFD());
                    Delete(processor);
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
                        case err_t::INITIATE_CLOSE:
                        default:
                            Delete(processor);
                            break;

                    }
                }
            }
        } // for(;;)
    } catch(listener_terminate_thread_t &terminateexception) {
        log<log_t::LISTENER_EXITING_THREAD>();
    }
}

} // namespace MMS::listener