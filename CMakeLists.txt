cmake_minimum_required(VERSION 3.15)

# Project information
project(Eclipse 
    VERSION 1.0.0
    DESCRIPTION "A minimal, colorful C++ logging library"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compile features for better compatibility
set(CMAKE_CXX_COMPILE_FEATURES cxx_std_17)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Platform-specific configurations
if(WIN32)
    if(MSVC)
        add_compile_options(/W4)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    else()
        add_compile_options(-Wall -Wextra -Wpedantic)
    endif()
elseif(UNIX)
    # Unix-like systems (Linux, macOS)
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(APPLE)
        # macOS-specific settings
        set(CMAKE_MACOSX_RPATH ON)
    endif()
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/Logger.cpp
)

# Header files
set(HEADERS
    include/Eclipse/Logger.h
    include/Eclipse/LogMacros.h
)

# Create the main library
add_library(EclipseCore STATIC ${SOURCES} ${HEADERS})

# Set target properties
set_target_properties(EclipseCore PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${HEADERS}"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Target compile features
target_compile_features(EclipseCore PUBLIC cxx_std_17)

# Include directories for the target
target_include_directories(EclipseCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Thread support (required for std::mutex, std::thread)
find_package(Threads REQUIRED)
target_link_libraries(EclipseCore PUBLIC Threads::Threads)

# Platform-specific linking
if(WIN32)
    # Windows doesn't need additional libraries for threading
elseif(UNIX)
    # Unix systems might need pthread explicitly
    target_link_libraries(EclipseCore PRIVATE pthread)
endif()

# Create an alias target for internal use
add_library(Eclipse::EclipseCore ALIAS EclipseCore)

# Interface library for header-only usage
add_library(Eclipse INTERFACE)
target_include_directories(Eclipse
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(Eclipse INTERFACE cxx_std_17)

# Create an alias for the interface
add_library(Eclipse::Eclipse ALIAS Eclipse)

# Optional: Build test executable
option(BUILD_TESTS "Build test executables" ON)
if(BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp")
    add_executable(EclipseTest test/main.cpp)
    target_link_libraries(EclipseTest PRIVATE EclipseCore)
    
    # Enable testing
    enable_testing()
    add_test(NAME EclipseBasicTest COMMAND EclipseTest)
endif()

# Installation configuration
include(GNUInstallDirs)

# Install the library
install(TARGETS EclipseCore Eclipse
    EXPORT EclipseTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Eclipse
)

# Install headers
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Create and install CMake config files
include(CMakePackageConfigHelpers)

# Generate the config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EclipseConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/EclipseConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Eclipse
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

# Generate the version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/EclipseConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install the targets export
install(EXPORT EclipseTargets
    FILE EclipseTargets.cmake
    NAMESPACE Eclipse::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Eclipse
)

# Install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/EclipseConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/EclipseConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Eclipse
)

# Create pkg-config file for systems that use it
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/eclipse.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/eclipse.pc"
    @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/eclipse.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Print configuration summary
message(STATUS "Eclipse Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
if(WIN32)
    message(STATUS "  Platform: Windows")
elseif(APPLE)
    message(STATUS "  Platform: macOS")
elseif(UNIX)
    message(STATUS "  Platform: Linux/Unix")
endif()