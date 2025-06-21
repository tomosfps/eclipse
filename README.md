# Eclipse

A minimal, colorful C++ logging library designed for use across multiple projects. Eclipse provides clean, structured log output with colored formatting, timestamps, and trace information to help you debug and monitor your applications effectively.

> [!CAUTION]
> Your terminal will need to support UTF8, otherwise some characters may not work.

## Features

Eclipse is a lightweight logging solution that offers:

- **Multiple Log Levels**: DEBUG, INFO, WARNING, ERROR with configurable filtering
- **Colored Output**: Different colors for each log level for better readability
- **Timestamps**: Automatic timestamp generation for each log entry
- **Trace Information**: File, line, and function information for debugging
- **Environment Configuration**: Set log levels via `.env` file
- **Singleton Pattern**: Global access without repeated instantiation
- **Cross-Platform**: Works on Windows, Linux, and macOS

## Features 
___

- [x] Colored output for different log levels
- [x] Timestamp generation
- [x] Environment variable configuration
- [x] Trace information (file, line, function)
- [x] Singleton pattern for global access
- [x] Thread-safe logging

## Build

If you'd like to build the project yourself, you'll need CMake and a C++17 compatible compiler:

1. Clone the repository:
   ```bash
   git clone https://github.com/tomosfps/eclipse.git
   cd eclipse
   ```

2. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

3. Configure the project:
   ```bash
   # Basic configuration (Debug is default)
   cmake ..
   
   # Specify build type:
   cmake -DCMAKE_BUILD_TYPE=Debug ..         # Debug build with symbols
   cmake -DCMAKE_BUILD_TYPE=Release ..       # Optimized release build
   cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. # Release with debug symbols
   cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..    # Size-optimized release
   
   # Additional options:
   cmake -DBUILD_TESTS=OFF ..               # Disable tests
   cmake -DCMAKE_INSTALL_PREFIX=/custom/path .. # Custom install location
   ```

4. Build the library:
   ```bash
   cmake --build .
   ```

### Global Installation

To install Eclipse globally for use in other projects:

1. Complete the build steps above

2. Install the library:

    - **On Linux/macOS:**
      ```bash
      sudo cmake --install .
      ```

    - **On Windows (from the build directory):**
      ```powershell
      cmake --install . --config Release
      ```
      > **Note:** You may need to run your terminal as Administrator if installing to a system location.

3. In your project's `CMakeLists.txt`:
   ```cmake
   cmake_minimum_required(VERSION 3.15)
   project(YourProject)

   find_package(Eclipse REQUIRED)

   add_executable(${PROJECT_NAME} main.cpp)
   target_link_libraries(${PROJECT_NAME} PRIVATE Eclipse::EclipseCore)
   ```

## Usage

### Basic Usage with Macros (Recommended)

The easiest way to use Eclipse is through the provided macros:

```cpp
#include <Eclipse/LogMacros.h>

int main() {
    LOG_INFO("APP", "Application started successfully");
    LOG_WARNING("CONFIG", "Using default configuration");
    LOG_ERROR("DATABASE", "Failed to connect to database");
    LOG_DEBUG("NETWORK", "Sending HTTP request");
    LOG_INFO_DETAILS("USER", "User logged in", "Username: john_doe");
    LOG_ERROR_DETAILS("AUTH", "Authentication failed", "Invalid credentials provided");
    
    return 0;
}
```

### Direct Logger Usage

You can also use the Logger class directly:

```cpp
#include <Eclipse/Logger.h>

int main() {
    Logger& logger = Logger::getInstance();
    
    // Set log level (optional, default is INFO)
    logger.setLogLevel(LogLevel::DEBUG);
    
    // Note: Direct usage requires implementing the friend functions
    // or using the macro interface shown above
    
    return 0;
}
```

### Environment Configuration

Create a `.env` file in your project root to configure the log level:

```env
LOG_LEVEL=DEBUG
```

Supported values:
- `DEBUG` or `0` - Show all messages
- `INFO` or `1` - Show info, warning, and error messages (default)
- `WARNING` or `2` - Show warning and error messages only
- `ERROR` or `3` - Show error messages only

### Log Output Format

Eclipse produces clean, structured output like this:

```
[2025-06-21 14:30:15] info: ┏ [APP] Application started successfully
  ┃ at main.cpp:10 [main]

[2025-06-21 14:30:15] warn: ┏ [CONFIG] Using default configuration  
  ┃ at main.cpp:11 [main]

[2025-06-21 14:30:15] error: ┏ [AUTH] Authentication failed
  ┃ at auth.cpp:25 [authenticate]
  ┗ [1] Invalid credentials provided
```

## Available Log Levels

| Level   | Color | Description |
|---------|-------|-------------|
| DEBUG   | Cyan  | Detailed debugging information |
| INFO    | Green | General information messages |
| WARNING | Yellow| Warning messages for potential issues |
| ERROR   | Red   | Error messages for serious problems |

## Integration Examples

### Simple Application

```cpp
#include <Eclipse/LogMacros.h>

int main() {
    LOG_INFO("STARTUP", "Initializing application");
    
    try {
        // Your application logic
        LOG_DEBUG("LOGIC", "Processing data");
    } catch (const std::exception& e) {
        LOG_ERROR_DETAILS("EXCEPTION", "Unhandled exception", e.what());
        return 1;
    }
    
    LOG_INFO("SHUTDOWN", "Application finished successfully");
    return 0;
}
```

### Multiple Files

**main.cpp**
```cpp
#include <Eclipse/LogMacros.h>
#include "utils.h"

int main() {
    LOG_INFO("MAIN", "Starting application");
    processData();
    LOG_INFO("MAIN", "Application completed");
    return 0;
}
```

**utils.h**
```cpp
#pragma once
void processData();
```

**utils.cpp**
```cpp
#include "utils.h"
#include <Eclipse/LogMacros.h>

void processData() {
    LOG_DEBUG("UTILS", "Processing data started");
    // Your processing logic here
    LOG_DEBUG("UTILS", "Processing data completed");
}
```

## Requirements

- C++17 compatible compiler
- CMake 3.15 or higher
- Standard C++ library

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
