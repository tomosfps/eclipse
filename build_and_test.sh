#!/bin/bash

# Eclipse Logger Build and Test Script
# This script builds the project and runs all tests

set -e  # Exit on any error

echo "=== Eclipse Logger Build and Test Script ==="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[STATUS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Create build directory
print_status "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
print_status "Configuring with CMake..."
cmake .. -DBUILD_TESTS=ON -DRUN_TESTS_BEFORE_BUILD=ON

# Build the project (this will run tests first due to our CMake configuration)
print_status "Building project (tests will run automatically before library build)..."
cmake --build . --config Release

# Run tests explicitly for verification
print_status "Running tests explicitly for verification..."
ctest --output-on-failure --verbose

# Check if all tests passed
if [ $? -eq 0 ]; then
    print_status "All tests passed! Build completed successfully."
    echo ""
    echo "=== Build Artifacts ==="
    echo "Library: $(pwd)/libEclipseCore.a"
    echo "Test executables:"
    ls -la tests/test_* 2>/dev/null || echo "  No test executables found"
else
    print_error "Some tests failed! Build may have issues."
    exit 1
fi

echo ""
print_status "Build and test process completed successfully!"
