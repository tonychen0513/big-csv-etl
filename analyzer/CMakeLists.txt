cmake_minimum_required(VERSION 2.6)

# Compile and Link analyzor

file( GLOB SOURCE_FILE "*.cpp")
 
add_executable(analyzor ${SOURCE_FILE})
target_compile_options(analyzor PRIVATE)
set_target_properties(analyzor PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_link_libraries(analyzor pthread)