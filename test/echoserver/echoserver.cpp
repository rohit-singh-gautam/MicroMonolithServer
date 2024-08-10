/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include <iostream>
#include <rohit/event/listner.h>
#include <rohit/net/tcpserverevent.h>
#include <thread>

std::unique_ptr<std::thread> plog_thread { };
bool log_thread_running = false;

static void log_thread_function() {
    constexpr auto wait_time = std::chrono::milliseconds(200);
    log_thread_running = true;
    while(log_thread_running) {
        std::this_thread::sleep_for(wait_time);
        rohit::logger::all.flush();
    }

    rohit::logger::all.flush();
}

void init_log_thread(const std::filesystem::path &filename) {
    if (!std::filesystem::exists(filename)) {
        auto parent { filename.parent_path() };
        std::filesystem::create_directories(parent);
    }
    int log_filedescriptor = open(filename.c_str(), O_RDWR | O_APPEND | O_CREAT, O_SYNC | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if ( log_filedescriptor < 0 ) {
        std::cerr << "Failed to open file " << filename.c_str() << ", error " << errno << ", " << strerror(errno) << std::endl;
    }

    rohit::logger::all.set_fd(log_filedescriptor);

    plog_thread.reset(new std::thread { log_thread_function });
}

class echoserver_t : public rohit::event::protocol_implementation_t {
    rohit::event::buffer_t localbuffer { };
public:
    void ProcessRead(const uint8_t *buffer, const size_t size, rohit::event::writer_t &writer) {
        writer.Write<true>(buffer, size, 0);
    }
};

thread_local rohit::event::buffer_t rohit::event::tcp::connection_t::tempbuffer { };

class ecchoservercreator_t : public rohit::event::protocol_implementation_creator_t {

public:

    rohit::event::protocol_implementation_t *create_protocol_implementation() override {
        return new echoserver_t();
    }

};

int main(int, char *[]) {
    rohit::log<rohit::log_t::APPLICATION_STARTING>();

    const std::filesystem::path filename("/tmp/iotcloud/log/deviceserver.log");
    init_log_thread(filename);

    ecchoservercreator_t echoservercreator { };
    rohit::event::listner_t locallistner { };
    rohit::event::tcp::server_t server { 4833, echoservercreator, &locallistner };
    locallistner.add(server);
    locallistner.multithread_loop(16);

    plog_thread->join();

    return 0;
}