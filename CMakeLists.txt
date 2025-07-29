
cmake_minimum_required(VERSION 3.15)
project(TaskScheduler VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set clang-tidy
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-format-style=file;-checks=*")

# Set clang-format
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
            "${CMAKE_SOURCE_DIR}/src/*.cpp"
            "${CMAKE_SOURCE_DIR}/include/*.h"
    )
    add_custom_target(format
            COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_SOURCE_FILES}
            COMMENT "Running clang-format on source files"
    )
endif()

# Add source files
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Add executable
add_executable(TaskScheduler ${SOURCES})

# Modern include directories
target_include_directories(TaskScheduler PRIVATE ${CMAKE_SOURCE_DIR}/include)

set_target_properties(TaskScheduler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Compiler specific flags
if(MSVC)
    message("Using MSVC compiler")
    target_compile_options(TaskScheduler PRIVATE /W4 /WX)
elseif(MINGW)
    message("Using MinGW compiler")
    target_compile_options(TaskScheduler PRIVATE -Wall -Wextra -Werror)
endif()