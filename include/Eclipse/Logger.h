/**
 * @file Logger.h
 * @brief Eclipse Logging Library - Core logging functionality
 * @author tomosfps
 * @date 2025
 * @version 1.0.2
 */

#pragma once
#include <string>
#include <mutex>

/**
 * @brief Enumeration of available log levels
 * 
 * Log levels are ordered by severity, with DEBUG being the lowest
 * and ERROR being the highest. NONE can be used to disable all logging.
 */
enum class LogLevel {
    DEBUG,      ///< Detailed information for debugging purposes
    INFO,       ///< General information about program execution
    WARN,       ///< Warning messages for potentially harmful situations
    ERR,        ///< Error messages for serious problems
    NONE        ///< No logging (disables all log output)
};

/**
 * @brief Thread-safe singleton logger class for structured logging
 * 
 * The Logger class provides a centralized logging system with colored output,
 * timestamps, and structured formatting using Unicode box-drawing characters.
 * It implements the singleton pattern to ensure global access and consistent
 * configuration across an application.
 * 
 * Features:
 * - Thread-safe operations using mutexes
 * - Colored output based on log levels
 * - Structured output with box-drawing characters (┏┃┗)
 * - Environment-based configuration via .env files
 * - Multiple log levels with filtering
 * 
 * @note This class is designed to be used through the LOG_* macros defined
 *       in LogMacros.h for convenience and automatic trace information.
 * 
 * @example
 * @code
 * Logger& logger = Logger::getInstance();
 * logger.setLogLevel(LogLevel::DEBUG);
 * // Use through macros: LOG_INFO("TAG", "Message");
 * @endcode
 */
class Logger {
public:
    /**
     * @brief Get the singleton instance of the Logger
     * 
     * This method implements thread-safe lazy initialization using std::call_once
     * to ensure only one Logger instance exists throughout the application lifetime.
     * 
     * @return Reference to the singleton Logger instance
     * @note This method is thread-safe
     */
    static Logger& getInstance();
    
    /**
     * @brief Set the minimum log level for output filtering
     * 
     * Only log messages with a level greater than or equal to the set level
     * will be displayed. This allows runtime filtering of log output.
     * 
     * @param level The minimum LogLevel to display
     * @note This method is thread-safe
     * 
     * @example
     * @code
     * logger.setLogLevel(LogLevel::WARNING); // Only show WARNING and ERROR
     * @endcode
     */
    void setLogLevel(LogLevel level);
    
    /**
     * @brief Get the current minimum log level
     * 
     * @return The current LogLevel threshold
     * @note This method is thread-safe
     */
    LogLevel getLogLevel() const;

private:
    /**
     * @brief Private constructor for singleton pattern
     * 
     * Initializes the logger with INFO level as default and loads
     * configuration from environment files if available.
     */
    Logger();
    
    /**
     * @brief Load log level configuration from .env file
     * 
     * Searches for LOG_LEVEL setting in a .env file in the current directory.
     * Supports both string values (DEBUG, INFO, WARNING, ERROR) and
     * numeric values (0-3).
     * 
     * @note This method is called automatically during initialization
     */
    void loadEnvLogLevel();

    /**
     * @brief Internal logging implementation with formatting and output
     * 
     * This method performs the actual logging work including level filtering,
     * formatting with colors and Unicode characters, and thread-safe output.
     * 
     * @param level The severity level of this log message
     * @param tag A categorization tag for the message (e.g., "HTTP", "DB")
     * @param message The main log message content
     * @param details Optional additional details or context
     * @param trace Optional trace information (file, line, function)
     * 
     * @note This method is thread-safe and handles all output formatting
     */
    void logInternal(LogLevel level, const std::string& tag, const std::string& message,
                     const std::string& details, const std::string& trace);

    /**
     * @brief Log a debug message
     * @param tag Message category tag
     * @param msg Main message content  
     * @param details Additional details (optional)
     * @param trace Trace information (optional)
     */
    void debug(const std::string& tag, const std::string& msg, const std::string& details, const std::string& trace);
    
    /**
     * @brief Log an info message
     * @param tag Message category tag
     * @param msg Main message content
     * @param details Additional details (optional)
     * @param trace Trace information (optional)
     */
    void info(const std::string& tag, const std::string& msg, const std::string& details, const std::string& trace);
    
    /**
     * @brief Log a warning message
     * @param tag Message category tag
     * @param msg Main message content
     * @param details Additional details (optional)
     * @param trace Trace information (optional)
     */
    void warning(const std::string& tag, const std::string& msg, const std::string& details, const std::string& trace);
    
    /**
     * @brief Log an error message
     * @param tag Message category tag
     * @param msg Main message content
     * @param details Additional details (optional)
     * @param trace Trace information (optional)
     */
    void error(const std::string& tag, const std::string& msg, const std::string& details, const std::string& trace);

    /**
     * @brief Get ANSI color code for the specified log level
     * 
     * @param level The LogLevel to get color for
     * @return ANSI escape sequence string for the color
     * 
     * Color mapping:
     * - DEBUG: Cyan (\033[36m)
     * - INFO: Green (\033[32m)
     * - WARNING: Yellow (\033[33m)
     * - ERROR: Red (\033[31m)
     */
    std::string getColor(LogLevel level) const;
    
    /**
     * @brief Get string representation of log level
     * 
     * @param level The LogLevel to convert
     * @return String name of the log level (e.g., "info", "error")
     */
    std::string getLevelName(LogLevel level) const;
    
    /**
     * @brief Generate current timestamp string
     * 
     * @return Formatted timestamp string in "YYYY-MM-DD HH:MM:SS" format
     * @note Uses platform-specific time functions for thread safety
     */
    std::string getTimestamp() const;
    
    /**
     * @brief Extract filename from full path in trace information
     * 
     * Processes trace strings to extract just the filename instead of the full path.
     * Handles both Windows (\) and Unix (/) path separators.
     * 
     * @param trace The trace string containing "at path:line [function]"
     * @return Modified trace string with shortened filename
    */
    std::string extractFilename(const std::string& trace) const;

    LogLevel currentLevel = LogLevel::DEBUG;    ///< Current minimum log level threshold
    mutable std::mutex logMutex;                ///< Mutex for thread-safe log output
    mutable std::mutex levelMutex;              ///< Mutex for thread-safe level access

    // Friend declarations for macro implementation functions
    friend void LOG_DEBUG_IMPL(const std::string&, const std::string&, const std::string&, const std::string&);   ///< Friend for debug macro
    friend void LOG_INFO_IMPL(const std::string&, const std::string&, const std::string&, const std::string&);    ///< Friend for info macro
    friend void LOG_WARNING_IMPL(const std::string&, const std::string&, const std::string&, const std::string&); ///< Friend for warning macro
    friend void LOG_ERROR_IMPL(const std::string&, const std::string&, const std::string&, const std::string&);   ///< Friend for error macro
};
