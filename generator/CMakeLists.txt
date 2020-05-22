cmake_minimum_required(VERSION 2.6)

# Compile and Link generator

file( GLOB SOURCE_FILE "*.cpp")
 
add_executable(generator ${SOURCE_FILE})
target_compile_options(generator PRIVATE -Werror -Wall -Wextra )
set_target_properties(generator PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_link_libraries(generator)