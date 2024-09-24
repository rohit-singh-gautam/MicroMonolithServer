cmake_minimum_required(VERSION 3.28)

add_executable(mmsclient mmsclient.cpp)

include_directories(${GLOBAL_INCLUDE} 
    ${CMAKE_SOURCE_DIR}/library/server/include
    ${CMAKE_SOURCE_DIR}/library/http/include
    ${CMAKE_SOURCE_DIR}/library/json/include)

target_link_libraries(mmsclient PUBLIC corelib httpparserlib)