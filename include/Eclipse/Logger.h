/**
 * @file Logger.h
 * @brief Eclipse Logging Library - Core logging functionality
 * @author tomosfps
 * @date 2025
 * @version 2.0.0
 */

#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <fstream>

namespace Eclipse
{
    /**
     * @brief Enumeration of available logging levels
     *
     * Defines the severity levels for log messages, from most verbose (DEBUG)
     * to most critical (FATAL). NONE can be used to disable all logging.
     */
    enum class ELevel
    {
        ECLIPSE_DEBUG, ///< Debug level - most verbose, for detailed diagnostic information
        ECLIPSE_INFO,  ///< Information level - general informational messages
        ECLIPSE_WARN,  ///< Warning level - potentially harmful situations
        ECLIPSE_ERROR, ///< Error level - error events that might still allow the application to continue
        ECLIPSE_FATAL, ///< Fatal level - very severe error events that will presumably lead to application abort
        ECLIPSE_NONE   ///< No logging - disables all log output
    };

    /**
     * @brief Enumeration of available output destinations
     *
     * Defines where log messages should be written - console, file, both, or neither.
     */
    enum class EOutput
    {
        CONSOLE, ///< Output to console/terminal only
        FILE,    ///< Output to log file only
        BOTH,    ///< Output to both console and file
        NONE     ///< No output - suppress all log messages
    };

    /**
     * @brief Singleton logger class providing thread-safe logging functionality
     *
     * The Logger class implements the singleton pattern to provide a global logging
     * interface. It supports multiple output destinations, configurable log levels,
     * file output, and thread-safe operations.
     *
     * @note This class is thread-safe and can be safely used from multiple threads.
     */
    class Logger
    {
    public:
        /**
         * @brief Get the singleton instance of the Logger
         *
         * @return Logger& Reference to the singleton Logger instance
         */
        static Logger &getInstance();

        /**
         * @brief Set the minimum logging level
         *
         * Only messages at or above this level will be logged.
         *
         * @param level The minimum logging level to set
         */
        void setLevel(ELevel level);

        /**
         * @brief Get the current logging level
         *
         * @return ELevel The current minimum logging level
         */
        ELevel getLevel() const;

        /**
         * @brief Set the log file path for file output
         *
         * Sets the file path where log messages will be written when file output
         * is enabled. Creates the file if it doesn't exist.
         *
         * @param filePath Path to the log file
         */
        void setLogFile(const std::string &filePath);

        /**
         * @brief Close the current log file
         *
         * Closes the log file stream if it's currently open. This is automatically
         * called when the logger is destroyed.
         */
        void closeLogFile();

        /**
         * @brief Set the output destination for log messages
         *
         * @param output The output destination (CONSOLE, FILE, BOTH, or NONE)
         */
        void setOutputDestination(EOutput output);

        /**
         * @brief Get the current output destination
         *
         * @return EOutput The current output destination setting
         */
        EOutput getOutputDestination() const;

        /**
         * @brief Load logger configuration from a file
         *
         * Loads logging configuration from a configuration file. The file should
         * contain key-value pairs for various logging settings.
         *
         * @param configPath Path to the configuration file
         * @return bool True if the configuration was loaded successfully, false otherwise
         */
        bool loadConfig(const std::string &configPath);

        /**
         * @brief Get the string representation of a logging level
         *
         * @param level The logging level to convert
         * @return std::string String representation of the level (e.g., "DEBUG", "INFO")
         */
        std::string getLevelName(ELevel level) const;

        /**
         * @brief Log a message with specified level and details
         *
         * The main logging function that outputs a formatted log message to the
         * configured destination(s). Thread-safe.
         *
         * @param level The severity level of the message
         * @param tag A tag or category for the message (e.g., "Database", "Network")
         * @param msg The main log message
         * @param details Additional details as a vector of strings
         * @param trace Stack trace or additional trace information
         */
        void log(ELevel level, const std::string &tag, const std::string &msg,
                 const std::vector<std::string> &details, const std::string &trace);

        /**
         * @brief Assert a condition and log an error if it fails
         *
         * Checks a boolean condition and logs an error message if the condition
         * is false. Useful for runtime assertions with logging.
         *
         * @param condition The condition to check
         * @param tag A tag or category for the assertion
         * @param msg The error message to log if the assertion fails
         * @param details Additional details (optional)
         * @param trace Stack trace information (optional)
         * @return bool The value of the condition parameter
         */
        bool assert(bool condition, const std::string &tag, const std::string &msg,
                    const std::vector<std::string> &details = {}, const std::string &trace = "");

        /**
         * @brief Get current timestamp as formatted string
         *
         * @return std::string Current timestamp in a readable format
         */
        std::string getTimestamp() const;

    private:
        /**
         * @brief Private constructor for singleton pattern
         *
         * Initializes the logger with default settings. Only accessible internally.
         */
        Logger();

        /**
         * @brief Destructor (defaulted)
         *
         * Automatically closes log files and cleans up resources.
         */
        ~Logger() = default;

        static Logger *instance; ///< Singleton instance pointer

        /**
         * @brief Get ANSI color code for a logging level
         *
         * @param level The logging level
         * @return std::string ANSI color escape sequence for console output
         */
        std::string getColour(ELevel level) const;

        /**
         * @brief Truncate file path to show only relevant parts
         *
         * Shortens long file paths for cleaner log output.
         *
         * @param path The full file path
         * @return std::string Truncated path
         */
        std::string truncatePath(const std::string &path) const;

        /**
         * @brief Parse string value to logging level enum
         *
         * @param value String representation of the level
         * @param level Reference to store the parsed level
         * @return bool True if parsing was successful, false otherwise
         */
        bool parseLevel(const std::string &value, ELevel &level) const;

        ELevel currentLevel = ELevel::ECLIPSE_DEBUG; ///< Current minimum logging level
        mutable std::mutex logMutex;                 ///< Mutex for thread-safe logging operations
        mutable std::mutex levelMutex;               ///< Mutex for thread-safe level operations
        mutable std::mutex fileMutex;                ///< Mutex for thread-safe file operations

        EOutput outputDestination = EOutput::CONSOLE; ///< Current output destination setting
        std::string logFilePath;                      ///< Path to the current log file
        std::ofstream logFileStream;                  ///< File stream for log file output
    };

    /**
     * @brief Utility function to create a details vector from a single string
     *
     * Helper function that converts a single string into a vector of strings
     * for use with the log function's details parameter.
     *
     * @param details Single detail string to convert
     * @return std::vector<std::string> Vector containing the single detail string
     */
    std::vector<std::string> eclipse_make_details(const std::string &details);
}