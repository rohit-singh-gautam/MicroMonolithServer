cmake_minimum_required(VERSION 3.28)

find_package(GTest CONFIG REQUIRED)

add_library(restlib src/rest.cpp)
target_link_libraries(restlib PUBLIC corelib)

set(HTTP_INCLUDE PUBLIC 
    ${GLOBAL_INCLUDE}
    ${CMAKE_SOURCE_DIR}/library/server/include
    ${CMAKE_SOURCE_DIR}/library/http/include
    ${CMAKE_SOURCE_DIR}/library/REST/include
)

set(CTEST_OUTPUT_ON_FAILURE 1)

add_executable(rest_test src/resttest.cpp)
include_directories(${HTTP_INCLUDE})
target_link_libraries(rest_test PRIVATE GTest::gtest_main restlib)

add_test(rest_test rest_test)
