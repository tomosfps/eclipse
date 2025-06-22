cmake_minimum_required(VERSION 3.15)

# Test configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Test executables
set(TEST_SOURCES
    test_logger.cpp
    test_config_edge_cases.cpp
    test_thread_safety.cpp
)

# Create test executables
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    
    # Link against the main library
    target_link_libraries(${TEST_NAME} PRIVATE EclipseCore)
    target_link_libraries(${TEST_NAME} PRIVATE Threads::Threads)
    
    # Include directories
    target_include_directories(${TEST_NAME} PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
    
    # Add filesystem library for C++17 (if needed)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
        target_link_libraries(${TEST_NAME} PRIVATE stdc++fs)
    endif()
    
    # Register as a test
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    
    # Set test properties
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()

# Create a comprehensive test that runs all tests
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS test_logger test_config_edge_cases test_thread_safety
    COMMENT "Running all Eclipse Logger tests"
)

# Test discovery for IDEs
include(GoogleTest OPTIONAL)
if(GTEST_FOUND)
    gtest_discover_tests(test_logger)
    gtest_discover_tests(test_config_edge_cases)
    gtest_discover_tests(test_thread_safety)
endif()
