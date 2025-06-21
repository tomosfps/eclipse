cmake_minimum_required(VERSION 3.17)
project(MyProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Eclipse package
find_package(Eclipse REQUIRED)

# Create your executable
add_executable(${PROJECT_NAME} main.cpp)

# Link against Eclipse
target_link_libraries(${PROJECT_NAME} PRIVATE Eclipse::EclipseCore)

# Alternative: If you want header-only usage (just for macros)
# target_link_libraries(${PROJECT_NAME} PRIVATE Eclipse::Eclipse)
