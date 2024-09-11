cmake_minimum_required(VERSION 3.28)

add_compile_options(-Wall -Werror -Wpedantic -Wextra -Weffc++ -fanalyzer -Wno-analyzer-malloc-leak -Wno-analyzer-use-of-uninitialized-value -Wno-analyzer-fd-leak -Wno-analyzer-fd-use-without-check)
#add_compile_options(-Wall -Werror -Wpedantic -Wextra -Weffc++ -fno-analyzer -Wno-analyzer-malloc-leak -Wno-analyzer-fd-leak)

add_executable(echoserver echoserver.cpp)
add_executable(echoclient echoclient.cpp)
add_executable(echoudpclient echoudpclient.cpp)
add_executable(echosslclient echosslclient.cpp)

include_directories(${GLOBAL_INCLUDE})

target_link_libraries(echoserver PUBLIC corelib)
target_link_libraries(echoclient PUBLIC corelib)
target_link_libraries(echoudpclient PUBLIC corelib)
target_link_libraries(echosslclient PUBLIC corelib)

file(COPY cert DESTINATION .)
