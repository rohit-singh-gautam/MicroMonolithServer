cmake_minimum_required(VERSION 3.28)

add_compile_options(-fno-analyzer -Wno-analyzer-malloc-leak)

find_package(GTest CONFIG REQUIRED)

add_library(corelib
    src/listener.cpp
    src/error.cpp
    src/configparser.cpp
    src/log.cpp
    src/tcpserverevent.cpp
)

include_directories(${GLOBAL_INCLUDE})

set(CTEST_OUTPUT_ON_FAILURE 1)

set(CTEST_OUTPUT_ON_FAILURE 1)

add_executable(core_test src/coretest.cpp)
target_link_libraries(core_test PRIVATE GTest::gtest_main)

add_test(core_test core_test)
