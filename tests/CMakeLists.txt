# Test configuration
enable_testing()

# Find required packages for testing
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Test 1: Basic Logging Test
add_executable(test_basic_logging test_basic_logging.cpp)
target_link_libraries(test_basic_logging Eclipse Threads::Threads)
target_include_directories(test_basic_logging PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Test 2: Multi-threaded Logging Test
add_executable(test_multithreaded_logging test_multithreaded_logging.cpp)
target_link_libraries(test_multithreaded_logging Eclipse Threads::Threads)
target_include_directories(test_multithreaded_logging PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Test 3: Configuration and File Logging Test
add_executable(test_config_file_logging test_config_file_logging.cpp)
target_link_libraries(test_config_file_logging Eclipse Threads::Threads)
target_include_directories(test_config_file_logging PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Test 4: Advanced Features Test
add_executable(test_advanced_features test_advanced_features.cpp)
target_link_libraries(test_advanced_features Eclipse Threads::Threads)
target_include_directories(test_advanced_features PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Add tests to CTest
add_test(NAME BasicLogging COMMAND test_basic_logging)
add_test(NAME MultithreadedLogging COMMAND test_multithreaded_logging)
add_test(NAME ConfigFileLogging COMMAND test_config_file_logging)
add_test(NAME AdvancedFeatures COMMAND test_advanced_features)

# Set test properties
set_tests_properties(BasicLogging PROPERTIES TIMEOUT 30)
set_tests_properties(MultithreadedLogging PROPERTIES TIMEOUT 60)
set_tests_properties(ConfigFileLogging PROPERTIES TIMEOUT 30)
set_tests_properties(AdvancedFeatures PROPERTIES TIMEOUT 45)

# Copy test configuration files to build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/demo.ini ${CMAKE_CURRENT_BINARY_DIR}/demo.ini COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/demo.env ${CMAKE_CURRENT_BINARY_DIR}/demo.env COPYONLY)
