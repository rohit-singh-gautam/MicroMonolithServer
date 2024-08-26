/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <repository.h>

int main(int argc, char *argv[]) {
    const std::filesystem::path filename("/tmp/iotcloud/log/deviceserver.log");
    if (argc != 1 && argc !=2) {
        std::cout << "Usage: " << argv[0] << " [configuration name]\n";
        return 0;
    }

    std::filesystem::path path { };
    if (argc == 1) {
        path = "conf/config.json";
    } else {
        path = argv[1];
    }

    MMS::listener::listener_t locallistener { 4,  filename };

    MMS::repository::Container repo {&locallistener, path};
    if (!repo.ReadConfigurations()) {
        std::cerr << "Unable to read configuration\n";
        return 0;
    }
    repo.AddServerToListener();
    
    locallistener.multithread_loop();
    locallistener.wait();

    return 0;
}