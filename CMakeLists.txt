cmake_minimum_required(VERSION 3.16)

# globally add MZ_DEBUG for the debug build
add_compile_definitions($<$<CONFIG:DEBUG>:MZ_DEBUG>)

project(mengze)

# create output folder
file(MAKE_DIRECTORY output)

# Add third party libraries
add_subdirectory(third_party)

add_subdirectory(mengze)

