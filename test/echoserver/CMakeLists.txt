cmake_minimum_required(VERSION 3.28)

add_compile_options(-Wall -Werror -Wpedantic -Wextra -Weffc++ -fno-analyzer -Wno-analyzer-malloc-leak -Wno-analyzer-fd-leak)

add_executable(echoserver echoserver.cpp)
add_executable(echoclient echoclient.cpp)

include_directories(${GLOBAL_INCLUDE})

target_link_libraries(echoserver PUBLIC corelib)
target_link_libraries(echoclient PUBLIC corelib)
