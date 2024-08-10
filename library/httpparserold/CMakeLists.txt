cmake_minimum_required(VERSION 3.28)

project(HTTPParser)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(OpenSSL REQUIRED)

add_compile_options(-Wall -Werror -Wpedantic -Wextra -Wno-effc++ -fno-analyzer)

make_directory(${PROJECT_BINARY_DIR}/include)

BISON_TARGET(HTTPParserBison
    ${PROJECT_SOURCE_DIR}/http11.yy
    ${PROJECT_BINARY_DIR}/http11parser.cpp
    DEFINES_FILE ${PROJECT_BINARY_DIR}/include/http11parser.h)
FLEX_TARGET(HTTPParserFlex
    ${PROJECT_SOURCE_DIR}/http11.l
    ${PROJECT_BINARY_DIR}/http11lexer.cpp)

ADD_FLEX_BISON_DEPENDENCY(HTTPParserFlex HTTPParserBison)

add_library(HTTPParser
    ${BISON_HTTPParserBison_OUTPUTS}
    ${FLEX_HTTPParserFlex_OUTPUTS}
    lib/http11driver.cpp
    lib/http11.cpp
    lib/hpack.cpp
    lib/http2.cpp
)

ADD_CUSTOM_COMMAND(TARGET HTTPParser
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy  ${PROJECT_BINARY_DIR}/location.hh ${PROJECT_BINARY_DIR}/include/location.hh
)

include_directories(HTTPParser PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}
    ${GLOBAL_INCLUDE}
    ${CMAKE_BINARY_DIR}/library/http/httpparser/include
    ${CMAKE_BINARY_DIR}/core/include
    ${CMAKE_SOURCE_DIR}/library/thirdparty/md5
)

target_link_libraries(HTTPParser PUBLIC
    OpenSSL::SSL
    OpenSSL::Crypto
)

