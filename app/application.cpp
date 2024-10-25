//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2024  Rohit Jairaj Singh (rohit@singh.org.in)          //
//                                                                      //
// This program is free software: you can redistribute it and/or modify //
// it under the terms of the GNU General Public License as published by //
// the Free Software Foundation, either version 3 of the License, or    //
// (at your option) any later version.                                  //
//                                                                      //
// This program is distributed in the hope that it will be useful,      //
// but WITHOUT ANY WARRANTY; without even the implied warranty of       //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        //
// GNU General Public License for more details.                         //
//                                                                      //
// You should have received a copy of the GNU General Public License    //
// along with this program.  If not, see <https://www.gnu.org/licenses/>//
//////////////////////////////////////////////////////////////////////////

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

    MMS::listener::listener_t locallistener { filename };

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