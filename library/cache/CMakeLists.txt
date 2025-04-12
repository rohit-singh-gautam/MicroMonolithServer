cmake_minimum_required(VERSION 3.28)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/library/cache/cacheinterface.h
    COMMAND serializer input ${CMAKE_SOURCE_DIR}/library/cache/cacheinterface.def output ${CMAKE_BINARY_DIR}/library/cache/cacheinterface.h
    DEPENDS ${CMAKE_SOURCE_DIR}/library/cache/cacheinterface.def serializer
    VERBATIM
)

find_package(GTest CONFIG REQUIRED)

add_library(cachelib src/cache.cpp)
target_link_libraries(cachelib PUBLIC corelib)

set(HTTP_INCLUDE PUBLIC 
    ${GLOBAL_INCLUDE}
    ${CMAKE_SOURCE_DIR}/library/server/include
    ${CMAKE_SOURCE_DIR}/library/http/include
    ${CMAKE_SOURCE_DIR}/library/REST/include
    ${CMAKE_SOURCE_DIR}/library/json/include
    ${CMAKE_SOURCE_DIR}/library/cache/include
    ${CMAKE_SOURCE_DIR}/library/lockfree/include
)

set(CTEST_OUTPUT_ON_FAILURE 1)

add_executable(cache_test src/cachetest.cpp)
include_directories(${HTTP_INCLUDE})
target_link_libraries(cache_test PRIVATE GTest::gtest_main cachelib)

add_test(cache_test cache_test)