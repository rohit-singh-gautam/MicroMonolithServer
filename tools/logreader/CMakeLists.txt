cmake_minimum_required(VERSION 3.28)

add_compile_options(-Wall -Werror -Wpedantic -Wextra -Weffc++ -fanalyzer -Wno-analyzer-malloc-leak)

add_executable(logreader logreader.cpp)

include_directories(${GLOBAL_INCLUDE})

target_link_libraries(logreader PUBLIC corelib)
