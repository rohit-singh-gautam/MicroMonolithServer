/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/log/log.h>
#include <mms/cmd/configparser.h>

#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>

void delete_log_file(const std::filesystem::path &path) {
    if (std::filesystem::exists(path)) {
        std::cout << "Deleting log file.\n";
        std::cout << "Remove '-c' option if you do not what to delete log file in future.\n";
        std::filesystem::remove(path);
    }
}

void wait_for_creation(const std::filesystem::path &path) {
    if (std::filesystem::exists(path)) return;
    std::cout << "Waiting till log file is created. \n";
    size_t count { 0 };
    while(!std::filesystem::exists(path)) {
        // Wait for one second
        using namespace std::chrono_literals;
        if (count >= 1) {
            std::cout << '.' << std::flush;
            count = 0;
        } else ++count;
        std::this_thread::sleep_for(1s);
    }
    std::cout << '\n';
}

int main(int argc, char *argv[]) {
    bool live;
    bool clean;
    bool display_version;
    std::filesystem::path log_file;
    MMS::commandline param_parser(
        "Parse and display logs",
        "Parse and display logs in sorted format",
        {
            {'l', "log_file", "file path", "Path to save log file", log_file, std::filesystem::path("/tmp/iotcloud/log/deviceserver.log")},
            {'w', "wait", "Wait mode will wait for more logs", live},
            {'c', "clean", "Delete log file", clean},
            {'v', "version", "Display version", display_version}
        }
    );

    if (!param_parser.parser(argc, argv)) {
        std::cout << param_parser.usage() << std::endl;
        return EXIT_SUCCESS;
    }

    if (display_version) {
        std::cout << param_parser.get_name() << " 1.0 "<< std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << "Log file path: " << log_file << '\n';

    if (clean) delete_log_file(log_file);

    if (live) wait_for_creation(log_file);
    else if (!std::filesystem::exists(log_file)) {
        std::cout << "Log file does not exists consider adding '-w' option if you want to wait for log file creation.\n";
        return 0;
    }

    MMS::logreader log_reader(log_file);

    while(true) {
        auto logstr = log_reader.readnextstring(live);
        if (logstr.empty()) break;
        std::cout << logstr << std::endl;
    }
    
    return 0;
}