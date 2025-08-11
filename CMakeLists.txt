
cmake_minimum_required(VERSION 3.15)
project(TaskScheduler VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the library with proper include directories
add_library(TaskSchedulerLib
    include/TaskScheduler.h
    include/Logger.h
    src/TaskScheduler.cpp
)

# Set include directories for the library
target_include_directories(TaskSchedulerLib PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Create executable and link to library
add_executable(TaskScheduler)

# Add all source files to executable (but link to library for headers)
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
target_sources(TaskScheduler PRIVATE ${SOURCES})

# Link the library to get proper include paths
target_link_libraries(TaskScheduler PRIVATE TaskSchedulerLib)

# Compiler specific flags
if(MSVC)
    message("Using MSVC compiler")
    target_compile_options(TaskScheduler PRIVATE /W4 /WX)
    target_compile_options(TaskSchedulerLib PRIVATE /W4 /WX)
elseif(MINGW)
    message("Using MinGW compiler")
    target_compile_options(TaskScheduler PRIVATE -Wall -Wextra -Werror)
    target_compile_options(TaskSchedulerLib PRIVATE -Wall -Wextra -Werror)
endif()

# Set target properties
set_target_properties(TaskScheduler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# For MSVC specific debug info
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /EHsc")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
endif()

# Google Test
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    enable_testing()
    
    include (FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )

     # Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Don't install gtest when this project is installed
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    # Make google test available
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests)
endif()

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

# Set clang-tidy
# set(CMAKE_CXX_CLANG_TIDY 
#     "clang-tidy;
#     -header-filter=${CMAKE_SOURCE_DIR}/.*;
#     -format-style=file;
#     -checks=*")