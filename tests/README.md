# Eclipse Logger Test Suite

This directory contains comprehensive tests for the Eclipse Logger library.

## Test Organization

### Core Tests (`test_logger.cpp`)
- **Singleton Pattern**: Verifies that `Logger::getInstance()` returns the same instance
- **Default Log Level**: Confirms default level is DEBUG
- **Log Level Management**: Tests setting/getting log levels
- **Configuration File Parsing**: Tests .env and .ini file parsing
- **Error Handling**: Tests graceful handling of invalid inputs
- **Macro Compilation**: Verifies all logging macros compile and execute
- **Log Level Filtering**: Tests that log messages are properly filtered by level

### Configuration Edge Cases (`test_config_edge_cases.cpp`)
- **Comments and Whitespace**: Tests parsing files with comments and extra whitespace
- **Multiple Sections**: Tests .ini files with multiple sections
- **Case Insensitive**: Tests case-insensitive log level parsing
- **Quoted Values**: Tests parsing of quoted configuration values
- **Invalid Values**: Tests handling of invalid log level values
- **Empty Values**: Tests handling of empty configuration values

### Thread Safety (`test_thread_safety.cpp`)
- **Concurrent Logging**: Tests multiple threads logging simultaneously
- **Concurrent Level Changes**: Tests thread safety of level changes
- **Singleton Thread Safety**: Verifies singleton pattern is thread-safe

## Running Tests

### Automatic Test Execution
Tests are automatically run before the main library is built when using CMake:

```bash
mkdir build && cd build
cmake .. -DBUILD_TESTS=ON -DRUN_TESTS_BEFORE_BUILD=ON
cmake --build .
```

### Manual Test Execution
To run tests manually:

```bash
# From the build directory
ctest --output-on-failure --verbose

# Or run individual tests
./tests/test_logger
./tests/test_config_edge_cases
./tests/test_thread_safety
```

### Using Build Scripts
Use the provided build scripts for convenience:

**Linux/macOS:**
```bash
./build_and_test.sh
```

**Windows:**
```batch
build_and_test.bat
```

## Test Configuration Options

### CMake Options
- `BUILD_TESTS=ON/OFF`: Enable/disable test building (default: ON)
- `RUN_TESTS_BEFORE_BUILD=ON/OFF`: Run tests before building main library (default: ON)

### Disabling Pre-Build Tests
To build without running tests first:

```bash
cmake .. -DBUILD_TESTS=ON -DRUN_TESTS_BEFORE_BUILD=OFF
cmake --build .
ctest  # Run tests after build
```

## Test Files Created
The tests create temporary configuration files during execution:
- `test_debug.env`, `test_info.env`: Test .env files
- `test_warn.ini`, `test_error.ini`: Test .ini files
- `test_numeric.env`: Test numeric log level format
- Various edge case test files

All test files are automatically cleaned up after test execution.

## Expected Test Output

### Successful Test Run
```
=== Eclipse Logger Test Suite ===

[TEST] Singleton Pattern: PASSED
[TEST] Default Log Level is DEBUG: PASSED
[TEST] Set/Get INFO Level: PASSED
...

=== Test Summary ===
Passed: 15/15
All tests PASSED!
```

### Failed Test Run
```
=== Eclipse Logger Test Suite ===

[TEST] Singleton Pattern: PASSED
[TEST] Default Log Level is DEBUG: FAILED
...

=== Test Summary ===
Passed: 14/15
Some tests FAILED!
```

## Integration with CI/CD

The test suite is designed to integrate easily with CI/CD systems:

1. **Exit Codes**: Tests return 0 for success, 1 for failure
2. **Verbose Output**: Detailed output for debugging failed tests
3. **Timeout Protection**: Tests have reasonable timeout limits
4. **No External Dependencies**: Tests use only standard C++ library features

## Adding New Tests

To add new tests:

1. Create a new `.cpp` file in the `tests/` directory
2. Follow the existing test pattern with a simple test runner
3. Add the new test file to `tests/CMakeLists.txt`
4. The test will automatically be included in the build process

### Test Template
```cpp
#include "Eclipse/Logger.h"
#include "Eclipse/LogMacros.h"
#include <iostream>

class MyTestRunner {
    // ... implement test runner pattern
};

int main() {
    MyTestRunner runner;
    
    // Add your tests here
    runner.test("Test Name", condition);
    
    return runner.runAll() ? 0 : 1;
}
```
