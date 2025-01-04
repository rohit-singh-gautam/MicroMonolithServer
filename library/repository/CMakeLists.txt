cmake_minimum_required(VERSION 3.28)

add_library(repositorylib src/repository.cpp)

include_directories(${GLOBAL_INCLUDE} 
    ${CMAKE_SOURCE_DIR}/library/server/include
    ${CMAKE_SOURCE_DIR}/library/http/include
    ${CMAKE_SOURCE_DIR}/library/json/include
    ${CMAKE_SOURCE_DIR}/library/cache/include
    ${CMAKE_SOURCE_DIR}/library/REST/include
    ${CMAKE_SOURCE_DIR}/library/lockfree/include
    ${CMAKE_SOURCE_DIR}/library/repository/include)

target_link_libraries(repositorylib PUBLIC corelib httpserverlib restlib cachelib)