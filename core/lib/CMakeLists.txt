cmake_minimum_required(VERSION 3.28)

add_compile_options(-Wall -Werror -Wpedantic -Wextra -Weffc++ -fno-analyzer -Wno-analyzer-malloc-leak)

add_library(corelib
    configparser.cpp
    log.cpp
)

include_directories(${GLOBAL_INCLUDE})