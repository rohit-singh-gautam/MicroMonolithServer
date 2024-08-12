/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/event/listener.h>


namespace MMS::event {

bool listener_t::created { false };

listener_t::listener_t(const std::filesystem::path &filename, const size_t max_event_epoll_return) : max_event_epoll_return { max_event_epoll_return } {
    if (created) {
        log<log_t::LISTENER_ALREADY_CREATED_FAILED>();
        throw listener_already_created_failed_t { };
    }
    created = true;

    init_log_thread(filename);

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        log<log_t::LISTENER_CREATE_FAILED>(errno);
        throw listener_create_failed_t { };
    }

    log<log_t::LISTENER_CREATE_SUCCESS>();
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
    while(true) {
        auto ret = epoll_wait(epollfd, events.get(), max_event_epoll_return, -1);

        if (ret == -1) {
            if (errno == EINTR || errno == EINVAL) {
                if (IsTerminated) {
                    pthread_exit(nullptr);
                }
            }

            log<log_t::LISTENER_LOOP_WAIT_INTERRUPTED>(errno);
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

            log<log_t::LISTENER_EVENT_RECEIVED>(processor->GetFD(), event.events);
            if ((event.events & EPOLLRDHUP)) {
                log<log_t::TCP_SERVER_PEER_CONNECTION_CLOSED>(processor->GetFD());
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

} // namespace MMS::event