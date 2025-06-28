/**
 * @file Logger.h
 * @brief Eclipse Logging Library - Core logging functionality
 * @author tomosfps
 * @date 2025
 * @version 1.0.3
 */

#pragma once
#include <string>
#include <mutex>

/**
 * @brief Enumeration of available log levels
 *
 * Log levels are ordered by severity, with ECLIPSE_DEBUG being the lowest
 * and ECLIPSE_ERROR being the highest. ECLIPSE_NONE can be used to disable all logging.
 */
enum class LogLevel
{
    ECLIPSE_DEBUG, ///< Detailed information for debugging purposes
    ECLIPSE_INFO,  ///< General information about program execution
    ECLIPSE_WARN,  ///< Warning messages for potentially harmful situations
    ECLIPSE_ERROR, ///< Error messages for serious problems
    ECLIPSE_NONE   ///< No logging (disables all log output)
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
 * - Environment-based configuration via .env and .ini files
 * - Multiple log levels with filtering
 * - Conflict-free ECLIPSE_* prefixed log levels
 *
 * @note This class is designed to be used through the LOG_* macros defined
 *       in LogMacros.h for convenience and automatic trace information.
 *
 * @example
 * @code
 * Logger& logger = Logger::getInstance();
 * logger.setLogLevel(LogLevel::ECLIPSE_DEBUG);
 * // Use through macros: LOG_INFO("TAG", "Message");
 * @endcode
 */
class Logger
{
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
    static Logger &getInstance();

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
     * logger.setLogLevel(LogLevel::ECLIPSE_WARN); // Only show ECLIPSE_WARN and ECLIPSE_ERROR
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

    /**
     * @brief Load configuration from a specific file path
     *
     * Allows loading log level configuration from a custom .env or .ini file.
     * Supports both absolute and relative paths. File type is determined by
     * extension (.env or .ini).
     *
     * @param configPath Path to the configuration file
     * @return true if configuration was loaded successfully, false otherwise
     *
     * @example
     * @code
     * Logger& logger = Logger::getInstance();
     * logger.loadConfigFromFile("./config/logging.ini");
     * @endcode
     */
    bool loadConfigFromFile(const std::string &configPath);

    /**
     * @brief Get the name of the current log level
     *
     * Returns a string representation of the current log level, e.g., "DEBUG", "INFO".
     * This can be useful for logging or displaying the current configuration.
     *
     * @return String name of the current log level
     */
    std::string getLogLevelName() const;

private:
    /**
     * @brief Private constructor for singleton pattern
     *
     * Initializes the logger with ECLIPSE_DEBUG level as default and loads
     * configuration from environment files if available.
     */
    Logger();

    /**
     * @brief Load log level configuration from configuration files
     *
     * Searches for LOG_LEVEL setting in .env or .ini files. If no custom path
     * is provided, looks for .env and .ini files in the current directory.
     * Falls back to ECLIPSE_DEBUG level if no configuration is found.
     *
     * Supports both string values (DEBUG, INFO, WARNING, ERROR) and
     * numeric values (0-3).
     *
     * @note This method is called automatically during initialization
     */
    void loadEnvLogLevel();

    /**
     * @brief Parse .env format configuration file
     *
     * Parses a .env format file looking for LOG_LEVEL=value lines.
     *
     * @param filePath Path to the .env file
     * @return true if LOG_LEVEL was found and parsed, false otherwise
     */
    bool parseEnvFile(const std::string &filePath);

    /**
     * @brief Parse .ini format configuration file
     *
     * Parses a .ini format file looking for LOG_LEVEL under [logging] section
     * or as a standalone LOG_LEVEL=value line.
     *
     * @param filePath Path to the .ini file
     * @return true if LOG_LEVEL was found and parsed, false otherwise
     */
    bool parseIniFile(const std::string &filePath);

    /**
     * @brief Convert string log level to LogLevel enum
     *
     * Converts string representations (both text and numeric) to LogLevel enum.
     *
     * @param levelStr String representation of log level
     * @param outLevel Reference to store the converted LogLevel
     * @return true if conversion was successful, false otherwise
     */
    bool parseLogLevelString(const std::string &levelStr, LogLevel &outLevel);

    /**
     * @brief Core logging implementation with formatting and thread-safe output
     *
     * This method performs the complete logging workflow:
     * 1. Checks if the message level meets the current threshold
     * 2. Formats the message with colors, timestamps, and Unicode box characters
     * 3. Outputs the structured log message to stdout
     *
     * The output format uses Unicode box-drawing characters (┏┃┗) to create
     * a structured appearance:
     * - ┏ (top-left corner) for the main message line
     * - ┃ (vertical line) for trace information
     * - ┗ (bottom-left corner) for additional details
     *
     * @param level Severity level of the log message
     * @param tag Category tag for message classification (e.g., "HTTP", "DB")
     * @param message Main content of the log message
     * @param details Optional additional context or details
     * @param trace Optional source location information (file:line [function])
     *
     * @note This method is thread-safe using two levels of mutex protection:
     *       - levelMutex for checking the current log level
     *       - logMutex for ensuring atomic output operations
     */
    void logInternal(LogLevel level, const std::string &tag, const std::string &message,
                     const std::string &details, const std::string &trace);

    /**
     * @brief Log a debug message
     * @param tag Message category tag
     * @param msg Main message content
     * @param details Additional details (optional)
     * @param trace Trace information (optional)
     */
    void debug(const std::string &tag, const std::string &msg, const std::string &details, const std::string &trace);

    /**
     * @brief Log an info message
     * @param tag Message category tag
     * @param msg Main message content
     * @param details Additional details (optional)
     * @param trace Trace information (optional)
     */
    void info(const std::string &tag, const std::string &msg, const std::string &details, const std::string &trace);

    /**
     * @brief Log a warning message
     * @param tag Message category tag
     * @param msg Main message content
     * @param details Additional details (optional)
     * @param trace Trace information (optional)
     */
    void warning(const std::string &tag, const std::string &msg, const std::string &details, const std::string &trace);

    /**
     * @brief Log an error message
     * @param tag Message category tag
     * @param msg Main message content
     * @param details Additional details (optional)
     * @param trace Trace information (optional)
     */
    void error(const std::string &tag, const std::string &msg, const std::string &details, const std::string &trace);

    /**
     * @brief Get ANSI color code for the specified log level
     *
     * @param level The LogLevel to get color for
     * @return ANSI escape sequence string for the color
     *
     * Color mapping:
     * - ECLIPSE_DEBUG: Cyan (\033[36m)
     * - ECLIPSE_INFO: Green (\033[32m)
     * - ECLIPSE_WARN: Yellow (\033[33m)
     * - ECLIPSE_ERROR: Red (\033[31m)
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
     * @brief Generate current timestamp in standardized format
     *
     * Creates a timestamp string representing the current system time
     * in "YYYY-MM-DD HH:MM:SS" format using platform-specific thread-safe
     * time conversion functions.
     *
     * @return Formatted timestamp string
     *
     * @note Uses platform-specific functions for thread safety:
     *       - Windows: localtime_s()
     *       - Unix/Linux: localtime_r()
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
    std::string extractFilename(const std::string &trace) const;

    LogLevel currentLevel = LogLevel::ECLIPSE_DEBUG; ///< Current minimum log level threshold (defaults to ECLIPSE_DEBUG)
    mutable std::mutex logMutex;                     ///< Mutex for thread-safe log output
    mutable std::mutex levelMutex;                   ///< Mutex for thread-safe level access

    // Friend declarations for macro implementation functions
    friend void LOG_DEBUG_IMPL(const std::string &, const std::string &, const std::string &, const std::string &);   ///< Friend for debug macro
    friend void LOG_INFO_IMPL(const std::string &, const std::string &, const std::string &, const std::string &);    ///< Friend for info macro
    friend void LOG_WARNING_IMPL(const std::string &, const std::string &, const std::string &, const std::string &); ///< Friend for warning macro
    friend void LOG_ERROR_IMPL(const std::string &, const std::string &, const std::string &, const std::string &);   ///< Friend for error macro
};
