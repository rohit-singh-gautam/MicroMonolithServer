cmake_minimum_required(VERSION 3.28)

add_compile_options(-Wno-analyzer-malloc-leak)

find_package(GTest CONFIG REQUIRED)

add_library(httpparserlib lib/parser.cpp lib/hpack.cpp lib/http2.cpp)
target_link_libraries(httpparserlib PUBLIC corelib)

set(HTTP_INCLUDE PUBLIC 
    ${GLOBAL_INCLUDE}
    ${CMAKE_SOURCE_DIR}/library/http/include
)

set(CTEST_OUTPUT_ON_FAILURE 1)

add_executable(http_parser_test lib/parsetest.cpp)
include_directories(${HTTP_INCLUDE})
target_link_libraries(http_parser_test PRIVATE GTest::gtest_main httpparserlib)

add_test(http_parser_test http_parser_test)
