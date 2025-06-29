
cmake_minimum_required(VERSION 3.30)
project(TaskScheduler VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add source files
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Add executable
add_executable(TaskScheduler ${SOURCES})

# Modern include directories
target_include_directories(TaskScheduler PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Compiler specific flags
if(MSVC)
    message("Using MSVC compiler")
    target_compile_options(TaskScheduler PRIVATE /W4 /WX)
elseif(MINGW)
    message("Using MinGW compiler")
    target_compile_options(TaskScheduler PRIVATE -Wall -Wextra -Werror)
endif()