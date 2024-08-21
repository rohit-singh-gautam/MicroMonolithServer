cmake_minimum_required(VERSION 3.28)

add_executable(httpserver httpserver.cpp)

include_directories(${GLOBAL_INCLUDE} ${CMAKE_SOURCE_DIR}/server/include ${CMAKE_SOURCE_DIR}/library/http/include)

target_link_libraries(httpserver PUBLIC corelib httpserverlib)

file(COPY www DESTINATION .)
file(COPY www1 DESTINATION .)
file(COPY cert DESTINATION .)
