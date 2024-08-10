cmake_minimum_required(VERSION 3.16)

project(countrycode)

# add_compile_options(-Wall -Werror -Wpedantic -Wextra -fanalyzer -Weffc++)
add_compile_options(-Wall -Werror -Wpedantic -Wextra -Weffc++ -fanalyzer -Wno-analyzer-malloc-leak)

add_executable(countrycode countrycode.cpp)

set_property(TARGET countrycode PROPERTY CXX_STANDARD 23)
