cmake_minimum_required(VERSION 3.28)

add_executable(mms application.cpp)

include_directories(${GLOBAL_INCLUDE} 
    ${CMAKE_SOURCE_DIR}/library/server/include
    ${CMAKE_SOURCE_DIR}/library/http/include
    ${CMAKE_SOURCE_DIR}/library/json/include
    ${CMAKE_SOURCE_DIR}/library/repository/include)

target_link_libraries(mms PUBLIC corelib httpserverlib repositorylib)

file(COPY conf DESTINATION .)
