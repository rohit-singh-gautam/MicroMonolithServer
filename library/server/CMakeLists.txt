cmake_minimum_required(VERSION 3.28)

find_package(GTest CONFIG REQUIRED)

add_library(httpserverlib src/http.cpp src/http1.cpp src/http2.cpp src/filehandler.cpp)
target_link_libraries(httpserverlib PUBLIC corelib httpparserlib)

include_directories(${GLOBAL_INCLUDE} ${CMAKE_SOURCE_DIR}/library/server/include ${CMAKE_SOURCE_DIR}/library/http/include)
