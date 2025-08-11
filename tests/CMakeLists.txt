
# Test Executables
add_executable(TaskSchedulerTests
    TaskSchedulerTests.cpp
)

# Link against Google Test libraries
target_link_libraries(TaskSchedulerTests
    PRIVATE
    GTest::gtest_main    
    TaskSchedulerLib
)

# Include directories (if not properly propagated from library)
target_include_directories(TaskSchedulerTests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Set target properties
set_target_properties(TaskSchedulerTests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF    
)

# Compiler-specific options for tests
if(MSVC)
    target_compile_options(TaskSchedulerTests PRIVATE /W4)
else()
    target_compile_options(TaskSchedulerTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Discover tests automatically
include(GoogleTest)
gtest_discover_tests(TaskSchedulerTests
    PROPERTIES
        TIMEOUT 30
    EXTRA_ARGS
        --gtest_output=xml:${CMAKE_BINARY_DIR}/test_results.xml
        --gtest_catch_exceptions=1
)