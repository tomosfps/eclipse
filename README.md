# Eclipse Logging Library

A lightweight, thread-safe C++ logging library designed for modern applications.

## Features

- **Thread-Safe**: Full thread safety for multi-threaded applications
- **Multiple Log Levels**: DEBUG, INFO, WARN, ERROR, FATAL, and NONE
- **Flexible Output**: Console, file, both, or no output
- **Configuration Files**: Load settings from INI-style configuration files
- **Convenient Macros**: Easy-to-use logging macros with automatic trace information
- **Cross-Platform**: Works on Windows, Linux, and macOS
- **Modern C++**: Built with C++17 standard
- **Singleton Pattern**: Global access with controlled instantiation
- **Colorized Output**: ANSI color codes for console output
- **Assertion Support**: Built-in assertion functionality with logging

## Quick Start

### Basic Usage

```cpp
#include "Eclipse/Logger.h"
#include "Eclipse/Macros.h"

int main() {
    // Get the logger instance
    Eclipse::Logger &logger = Eclipse::Logger::getInstance();
    
    // Set logging level
    logger.setLevel(Eclipse::ELevel::ECLIPSE_DEBUG);
    
    // Use convenient macros for logging
    ECLIPSE_INFO("App", "Application started");
    ECLIPSE_DEBUG("Database", "Connection established", "host=localhost");
    ECLIPSE_WARNING("Memory", "High memory usage", "usage=85%");
    ECLIPSE_ERROR("Network", "Connection failed", "timeout=30s");
    ECLIPSE_FATAL("System", "Critical failure", "component=core");
    
    return 0;
}
```

### File Logging

```cpp
#include "Eclipse/Logger.h"
#include "Eclipse/Macros.h"

int main() {
    Eclipse::Logger &logger = Eclipse::Logger::getInstance();
    
    // Set up file logging
    logger.setLogFile("application.log");
    logger.setOutputDestination(Eclipse::EOutput::BOTH); // Console and file
    
    ECLIPSE_INFO("System", "Logging to file enabled");
    
    // Close log file when done
    logger.closeLogFile();
    
    return 0;
}
```

### Configuration File

Create a configuration file (e.g., `config.ini`):

```ini
[application]
name=MyApp
version=1.0.0

[logging]
ECLIPSE_LOG_LEVEL=INFO

[database]
host=localhost
port=5432
```

Load configuration in your application:

```cpp
Eclipse::Logger &logger = Eclipse::Logger::getInstance();
bool success = logger.loadConfig("config.ini");
if (success) {
    ECLIPSE_INFO("Config", "Configuration loaded successfully");
}
```

## Building

### Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler (GCC, Clang, MSVC)
- Git (for cloning)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/tomosfps/eclipse.git
cd eclipse

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the library
cmake --build .

# Run tests
ctest
```

### Integration with CMake

Add Eclipse to your CMake project:

```cmake
# If Eclipse is a subdirectory in your project
add_subdirectory(eclipse)

# Link Eclipse to your target
target_link_libraries(your_target Eclipse)

# Include Eclipse headers
target_include_directories(your_target PRIVATE eclipse/include)
```

## API Reference

### Log Levels

| Level | Description |
|-------|-------------|
| `ECLIPSE_DEBUG` | Most verbose, detailed diagnostic information |
| `ECLIPSE_INFO` | General informational messages |
| `ECLIPSE_WARN` | Potentially harmful situations |
| `ECLIPSE_ERROR` | Error events that might allow application to continue |
| `ECLIPSE_FATAL` | Very severe errors that may cause application abort |
| `ECLIPSE_NONE` | Disables all logging |

### Output Destinations

| Destination | Description |
|-------------|-------------|
| `CONSOLE` | Output to console/terminal only |
| `FILE` | Output to log file only |
| `BOTH` | Output to both console and file |
| `NONE` | Suppress all log output |

### Core Methods

```cpp
// Get singleton instance
Logger& logger = Logger::getInstance();

// Set minimum log level
logger.setLevel(ELevel::ECLIPSE_INFO);

// Set output destination
logger.setOutputDestination(EOutput::BOTH);

// Set log file
logger.setLogFile("app.log");

// Load configuration
bool success = logger.loadConfig("config.ini");

// Manual logging
logger.log(ELevel::ECLIPSE_INFO, "tag", "message", details, trace);

// Assertions
bool result = logger.assert(condition, "tag", "message");
```

### Logging Macros

```cpp
// Basic logging macros (automatically include trace information)
ECLIPSE_DEBUG("tag", "message", "detail1", "detail2");
ECLIPSE_INFO("tag", "message");
ECLIPSE_WARNING("tag", "message", "additional_info");
ECLIPSE_ERROR("tag", "message", "error_code=500");
ECLIPSE_FATAL("tag", "message", "component=core");

// Assertion macro
ECLIPSE_ASSERT(ptr != nullptr, "Memory", "Null pointer detected", "variable=ptr");
```

## Configuration File Format

Eclipse supports INI-style configuration files with the following format:

```ini
[logging]
ECLIPSE_LOG_LEVEL=INFO    # Valid values: DEBUG, INFO, WARN, ERROR, FATAL, NONE

[application]
name=YourApp
version=1.0.0

# Other sections for your application configuration
[database]
host=localhost
port=5432
```

## Examples

### Multi-threaded Logging

```cpp
#include "Eclipse/Logger.h"
#include "Eclipse/Macros.h"
#include <thread>
#include <vector>

void worker_thread(int id) {
    for (int i = 0; i < 10; ++i) {
        ECLIPSE_INFO("Worker", "Processing task", 
                    "thread_id=" + std::to_string(id),
                    "task=" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    Eclipse::Logger &logger = Eclipse::Logger::getInstance();
    logger.setLevel(Eclipse::ELevel::ECLIPSE_DEBUG);
    
    std::vector<std::thread> threads;
    
    // Start multiple worker threads
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(worker_thread, i);
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    ECLIPSE_INFO("Main", "All threads completed");
    return 0;
}
```

### Advanced Configuration

```cpp
#include "Eclipse/Logger.h"
#include "Eclipse/Macros.h"
#include <chrono>
#include <iomanip>
#include <sstream>

class Application {
private:
    Eclipse::Logger &logger;
        
public:
    Application() : logger(Eclipse::Logger::getInstance()) {
        // Load configuration
        if (!logger.loadConfig("app.config")) {
            ECLIPSE_WARNING("Config", "Could not load config file, using defaults");
        }
        
        // Set up file logging with timestamp
        std::string logFile = "app_" + logger.getTimestamp() + ".log";
        logger.setLogFile(logFile);
        logger.setOutputDestination(Eclipse::EOutput::BOTH);
        
        ECLIPSE_INFO("App", "Application initialized", "config=loaded", "logging=enabled");
    }
    
    void run() {
        ECLIPSE_INFO("App", "Application starting");
        
        try {
            // Application logic here
            processData();
        } catch (const std::exception& e) {
            ECLIPSE_ERROR("App", "Exception caught", "what=" + std::string(e.what()));
        }
        
        ECLIPSE_INFO("App", "Application finished");
    }
    
private:
    void processData() {
        ECLIPSE_DEBUG("Processing", "Starting data processing");
        
        // Simulate some work with assertions
        std::vector<int> data;
        data.resize(1024);
        
        ECLIPSE_ASSERT(!data.empty(), "Memory", "Failed to allocate data buffer");
        
        if (!data.empty()) {
            ECLIPSE_DEBUG("Processing", "Data allocated successfully", "size=1024");
            // Process data...
            std::fill(data.begin(), data.end(), 42);
            ECLIPSE_DEBUG("Processing", "Data processing completed");
        }
    }
};
```

## Testing

The library includes comprehensive tests covering:

- Basic logging functionality
- Thread safety
- Configuration file loading
- File output
- Advanced features

Run tests with:

```bash
cd build
ctest --verbose
```

## Thread Safety

Eclipse is fully thread-safe and uses multiple mutexes to ensure safe concurrent access:

- `logMutex`: Protects logging operations
- `levelMutex`: Protects level changes
- `fileMutex`: Protects file operations

You can safely use Eclipse from multiple threads without additional synchronization.

## Performance Considerations

- The singleton pattern ensures minimal overhead
- File I/O is only performed when necessary
- Color codes are only applied for console output
- Log level filtering happens early to avoid unnecessary string operations

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

