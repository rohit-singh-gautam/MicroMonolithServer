cmake_minimum_required(VERSION 3.28)

add_compile_options(-fno-analyzer -Wno-analyzer-malloc-leak)

add_library(corelib
    src/listner.cpp
    src/error.cpp
    src/configparser.cpp
    src/log.cpp
    src/tcpserverevent.cpp
)

include_directories(${GLOBAL_INCLUDE})
