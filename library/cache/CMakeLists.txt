cmake_minimum_required(VERSION 3.28)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/library/cache/include/cacheinterface.h
    COMMAND serializer input ${CMAKE_SOURCE_DIR}/library/cache/src/cacheinterface.def output ${CMAKE_BINARY_DIR}/library/cache/include/cacheinterface.h
    DEPENDS ${CMAKE_SOURCE_DIR}/library/cache/src/cacheinterface.def serializer
    VERBATIM
)

find_package(GTest CONFIG REQUIRED)

set(HTTP_INCLUDE PUBLIC 
    ${GLOBAL_INCLUDE}
    ${CMAKE_SOURCE_DIR}/library/server/include
    ${CMAKE_SOURCE_DIR}/library/http/include
    ${CMAKE_SOURCE_DIR}/library/REST/include
    ${CMAKE_SOURCE_DIR}/library/json/include
    ${CMAKE_SOURCE_DIR}/library/cache/include
    ${CMAKE_SOURCE_DIR}/library/lockfree/include
    ${CMAKE_SOURCE_DIR}/library/Serializer/include
    ${CMAKE_BINARY_DIR}/library/cache/include
)
include_directories(${HTTP_INCLUDE})

add_library(cachelib ${CMAKE_BINARY_DIR}/library/cache/include/cacheinterface.h src/cache.cpp)
target_link_libraries(cachelib PUBLIC corelib)

set(CTEST_OUTPUT_ON_FAILURE 1)

add_executable(cache_test src/cachetest.cpp)
target_link_libraries(cache_test PRIVATE GTest::gtest_main cachelib)

add_test(cache_test cache_test)