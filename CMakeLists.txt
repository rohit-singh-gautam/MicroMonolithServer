cmake_minimum_required(VERSION 3.28)

project(MicroMonolithServer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#add_compile_options(-Wall -Werror -Wpedantic -Wextra -Weffc++ -fanalyzer)
add_compile_options(-Wall -Werror -Wpedantic -Wextra -Weffc++ -Wsuggest-override)

enable_testing()

set(GLOBAL_INCLUDE ${PROJECT_SOURCE_DIR}/core/include/)

add_subdirectory(core)
add_subdirectory(server)
add_subdirectory(library)
add_subdirectory(test/echoserver)
add_subdirectory(test/logreader)
