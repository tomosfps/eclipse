cmake_minimum_required(VERSION 3.16)
project(Eclipse VERSION 2.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/Od /Zi)
    else()
        add_compile_options(/O2)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3)
    endif()
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find required packages
find_package(Threads REQUIRED)

# Source files
set(ECLIPSE_SOURCES
    src/Logger.cpp
)

# Header files
set(ECLIPSE_HEADERS
    include/Eclipse/Logger.h
    include/Eclipse/Macros.h
)

# Create the Eclipse library
add_library(Eclipse STATIC ${ECLIPSE_SOURCES} ${ECLIPSE_HEADERS})

# Set target properties
set_target_properties(Eclipse PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${ECLIPSE_HEADERS}"
)

# Link libraries
target_link_libraries(Eclipse PRIVATE Threads::Threads)

# Include directories for the target
target_include_directories(Eclipse 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compile definitions
target_compile_definitions(Eclipse PRIVATE 
    $<$<CONFIG:Debug>:ECLIPSE_DEBUG_BUILD>
    $<$<CONFIG:Release>:ECLIPSE_RELEASE_BUILD>
)

# Enable testing
enable_testing()

# Add tests subdirectory
add_subdirectory(tests)

# Add a custom target to run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_basic_logging test_multithreaded_logging test_config_file_logging
    COMMENT "Running all Eclipse library tests"
)

# Installation
include(GNUInstallDirs)

install(TARGETS Eclipse
    EXPORT EclipseTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Eclipse
)

# Install headers
install(DIRECTORY include/Eclipse
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Create config files for find_package()
install(EXPORT EclipseTargets
    FILE EclipseTargets.cmake
    NAMESPACE Eclipse::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Eclipse
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/EclipseConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/EclipseConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Eclipse
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/EclipseConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/EclipseConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/EclipseConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Eclipse
)

# Package configuration
set(CPACK_PACKAGE_NAME "Eclipse")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Eclipse C++ Logging Library")
set(CPACK_PACKAGE_VENDOR "tomosfps")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

include(CPack)