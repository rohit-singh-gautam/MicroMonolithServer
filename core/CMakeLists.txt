cmake_minimum_required(VERSION 3.28)

add_compile_options(-fno-analyzer -Wno-analyzer-malloc-leak -Werror=analyzer-fd-leak)

find_package(GTest CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

add_library(corelib
    src/listener.cpp
    src/error.cpp
    src/configparser.cpp
    src/netbase.cpp
    src/log.cpp
    src/tcpserver.cpp
    src/tcpsslserver.cpp
    src/sslcommon.cpp
    src/udpserversimple.cpp
)

include_directories(${GLOBAL_INCLUDE})

target_link_libraries(corelib PUBLIC 
    OpenSSL::SSL
    OpenSSL::Crypto
)


set(CTEST_OUTPUT_ON_FAILURE 1)

add_executable(core_test test/coretest.cpp test/streamtest.cpp)
target_link_libraries(core_test PRIVATE GTest::gtest_main)

add_test(core_test core_test)
