cmake_minimum_required(VERSION 3.28)

add_library(http lib/http.cpp)


include_directories(http PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}
    ${GLOBAL_INCLUDE}
    ${CMAKE_SOURCE_DIR}/core/include
    ${CMAKE_SOURCE_DIR}/library/http/include
    ${CMAKE_SOURCE_DIR}/library/thirdparty/md5
)
