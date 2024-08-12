cmake_minimum_required(VERSION 3.28)

add_compile_options(-Wno-analyzer-malloc-leak)

find_package(GTest CONFIG REQUIRED)


add_library(httplib lib/parser.cpp)

set(HTTP_INCLUDE PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}
    ${GLOBAL_INCLUDE}
    ${CMAKE_SOURCE_DIR}/core/include
    ${CMAKE_SOURCE_DIR}/library/http/include
    ${CMAKE_SOURCE_DIR}/library/thirdparty/md5
)

set(CTEST_OUTPUT_ON_FAILURE 1)

add_executable(http_parser_test lib/parsetest.cpp)
include_directories(${HTTP_INCLUDE} ${GTEST_INCLUDE_DIRS})
target_link_libraries(http_parser_test PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main httplib)

add_test(http_parser_test http_parser_test)
