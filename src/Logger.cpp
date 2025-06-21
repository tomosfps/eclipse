/**
 * @file Logger.cpp
 * @brief Eclipse Logging Library - Implementation of the Logger class
 * @author tomosfps
 * @date 2025
 * @version 1.0.2
 * 
 * This file contains the implementation of the Logger class, providing
 * thread-safe, colored logging with structured output formatting using
 * Unicode box-drawing characters.
 */

#include "Eclipse/Logger.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief Constructor - Initialize logger with default INFO level
 * 
 * Private constructor implementing singleton pattern. Sets the initial
 * log level to INFO and attempts to load configuration from environment.
 */
Logger::Logger() : currentLevel(LogLevel::INFO) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        DWORD mode;
        GetConsoleMode(hConsole, &mode);
        SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    loadEnvLogLevel();
}

/**
 * @brief Get singleton instance with thread-safe lazy initialization
 * 
 * Implements the singleton pattern using std::call_once for thread-safe
 * initialization. The instance is created only once and persists for
 * the lifetime of the application.
 * 
 * @return Reference to the singleton Logger instance
 * @note Thread-safe implementation using std::call_once
 */
Logger& Logger::getInstance() {
    static std::once_flag flag;
    static Logger* instance = nullptr;
    std::call_once(flag, []() {
        instance = new Logger();
    });
    return *instance;
}

/**
 * @brief Set the minimum log level threshold (thread-safe)
 * 
 * Updates the minimum log level required for messages to be displayed.
 * Messages with levels below this threshold will be filtered out.
 * 
 * @param level The new minimum LogLevel threshold
 * @note Thread-safe operation using mutex protection
 */
void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(levelMutex);
    currentLevel = level;
}

/**
 * @brief Get the current minimum log level threshold (thread-safe)
 * 
 * Returns the current log level threshold used for filtering messages.
 * 
 * @return Current LogLevel threshold
 * @note Thread-safe operation using mutex protection
 */
LogLevel Logger::getLogLevel() const {
    std::lock_guard<std::mutex> lock(levelMutex);
    return currentLevel;
}

/**
 * @brief Log a debug-level message
 * 
 * Forwards debug messages to the internal logging system with DEBUG level.
 * 
 * @param tag Category tag for message classification
 * @param msg Main message content
 * @param details Additional details or context
 * @param trace Source location information
 */
void Logger::debug(const std::string& tag, const std::string& msg, const std::string& details, const std::string& trace) {
    logInternal(LogLevel::DEBUG, tag, msg, details, trace);
}

/**
 * @brief Log an info-level message
 * 
 * Forwards info messages to the internal logging system with INFO level.
 * 
 * @param tag Category tag for message classification
 * @param msg Main message content
 * @param details Additional details or context
 * @param trace Source location information
 */
void Logger::info(const std::string& tag, const std::string& msg, const std::string& details, const std::string& trace) {
    logInternal(LogLevel::INFO, tag, msg, details, trace);
}

/**
 * @brief Log a warning-level message
 * 
 * Forwards warning messages to the internal logging system with WARNING level.
 * 
 * @param tag Category tag for message classification
 * @param msg Main message content
 * @param details Additional details or context
 * @param trace Source location information
 */
void Logger::warning(const std::string& tag, const std::string& msg, const std::string& details, const std::string& trace) {
    logInternal(LogLevel::WARN, tag, msg, details, trace);
}

/**
 * @brief Log an error-level message
 * 
 * Forwards error messages to the internal logging system with ERROR level.
 * 
 * @param tag Category tag for message classification
 * @param msg Main message content
 * @param details Additional details or context
 * @param trace Source location information
 */
void Logger::error(const std::string& tag, const std::string& msg, const std::string& details, const std::string& trace) {
    logInternal(LogLevel::ERR, tag, msg, details, trace);
}

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
void Logger::logInternal(LogLevel level, const std::string& tag, const std::string& message,
                         const std::string& details, const std::string& trace) {
    // First check: level filtering with separate lock scope
    {
        std::lock_guard<std::mutex> levelLock(levelMutex);
        if (level < currentLevel) return;
    }

    // Second lock: ensure atomic output formatting and writing
    std::lock_guard<std::mutex> logLock(logMutex);
    
    const std::string reset = "\033[0m";
    const std::string dim = "\033[90m";
    const std::string white = "\033[37m";
    std::ostringstream out;

    std::string timestamp = getTimestamp();
    std::string levelName = getLevelName(level);
    
    // Calculate padding for alignment - match the exact format of the main line
    std::string plainPrefix = "[" + timestamp + "] " + std::string(5, ' ') + ": ";
    std::string padding(plainPrefix.length(), ' ');

    // Main log line: [timestamp] level: ┏ [tag] message
    out << dim << "[" << timestamp << "] " << reset
        << getColor(level) << std::setw(5) << std::left << levelName << reset
        << white << ":" << reset
        << white << " ┏ [" << getColor(level) << tag << reset << "] " 
        << white << message << reset << "\n";

    // Optional trace/details lines with correct box-drawing characters
    if (!trace.empty() && details.empty()) {
        std::string cleanTrace = extractFilename(trace);
        size_t atPos = cleanTrace.find("at ");
        if (atPos != std::string::npos) {
            out << white << padding << " ┗ at " << dim 
                << cleanTrace.substr(atPos + 3) << reset << "\n";
        } else {
            out << white << padding << " ┗ " << dim << cleanTrace << reset << "\n";
        }
    } else {
        if (!trace.empty()) {
            std::string cleanTrace = extractFilename(trace);
            size_t atPos = cleanTrace.find("at ");
            if (atPos != std::string::npos) {
                out << white << padding << "┃ at " << dim 
                    << cleanTrace.substr(atPos + 3) << reset << "\n";
            } else {
                out << white << padding << "┃ " << dim << cleanTrace << reset << "\n";
            }
        }
        if (!details.empty()) {
            out << white << padding << "┗ [1] " << dim << details << reset << "\n";
        }
    }

    std::cout << out.str();
    std::cout.flush();
}

/**
 * @brief Extract filename from full path in trace information
 * 
 * Processes trace strings to extract just the filename instead of the full path.
 * Handles both Windows (\) and Unix (/) path separators.
 * 
 * @param trace The trace string containing "at path:line [function]"
 * @return Modified trace string with shortened filename
 */
std::string Logger::extractFilename(const std::string& trace) const {
    size_t atPos = trace.find("at ");
    if (atPos == std::string::npos) return trace;
    
    size_t pathStart = atPos + 3;
    size_t colonPos = trace.find(':', pathStart);
    if (colonPos == std::string::npos) return trace;
    std::string fullPath = trace.substr(pathStart, colonPos - pathStart);
    
    // Find last separator (\ or /)
    size_t lastSep = fullPath.find_last_of("\\/");
    std::string filename = (lastSep != std::string::npos) ? 
                          fullPath.substr(lastSep + 1) : fullPath;
    
    // Reconstruct the trace with just the filename
    return trace.substr(0, pathStart) + filename + trace.substr(colonPos);
}

/**
 * @brief Get ANSI color escape sequence for log level
 * 
 * Returns the appropriate ANSI escape sequence to colorize output
 * based on the severity level of the log message.
 * 
 * @param level The LogLevel to get color for
 * @return ANSI escape sequence string for terminal coloring
 * 
 * Color scheme:
 * - DEBUG: Cyan (\033[36m) - Cool color for detailed information
 * - INFO: Green (\033[32m) - Positive color for normal operation
 * - WARNING: Yellow (\033[33m) - Cautionary color for potential issues
 * - ERROR: Red (\033[31m) - Alert color for serious problems
 * - Default: Reset (\033[0m) - No color/default terminal color
 */
std::string Logger::getColor(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "\033[36m";      // Cyan
        case LogLevel::INFO:    return "\033[32m";      // Green
        case LogLevel::WARN:    return "\033[33m";      // Yellow
        case LogLevel::ERR:     return "\033[31m";      // Red
        default:                return "\033[0m";       // Reset/Default
    }
}

/**
 * @brief Get string representation of log level
 * 
 * Converts LogLevel enumeration values to their corresponding
 * lowercase string representations for display purposes.
 * 
 * @param level The LogLevel enumeration value
 * @return String name of the log level
 * 
 * @note Returns "unknown" for any unrecognized log level values
 */
std::string Logger::getLevelName(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "debug";
        case LogLevel::INFO:    return "info";
        case LogLevel::WARN:    return "warn";
        case LogLevel::ERR:     return "error";
        default:                return "unknown";
    }
}

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
std::string Logger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm buf{};
#ifdef _WIN32
    localtime_s(&buf, &in_time_t);
#else
    localtime_r(&in_time_t, &buf);
#endif
    std::ostringstream ss;
    ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

/**
 * @brief Load log level configuration from environment file
 * 
 * Attempts to read log level configuration from a ".env" file in the
 * current working directory. Supports both string-based and numeric
 * log level specifications.
 * 
 * Supported formats in .env file:
 * - String values: LOG_LEVEL=DEBUG, LOG_LEVEL=INFO, etc.
 * - Numeric values: LOG_LEVEL=0 (DEBUG), LOG_LEVEL=1 (INFO), etc.
 * 
 * The search stops at the first LOG_LEVEL= line found. If the file
 * doesn't exist or contains no valid LOG_LEVEL setting, the default
 * level (INFO) is retained.
 * 
 * @note This method is called automatically during Logger initialization
 * @note The level setting is applied with thread-safe mutex protection
 */
void Logger::loadEnvLogLevel() {
    std::ifstream envFile(".env");
    std::string line;
    
    // Search for LOG_LEVEL setting in .env file
    while (std::getline(envFile, line)) {
        if (line.find("LOG_LEVEL=") == 0) {
            std::string levelStr = line.substr(10);
            
            // Convert to uppercase for case-insensitive matching
            std::transform(levelStr.begin(), levelStr.end(), levelStr.begin(), ::toupper);

            // Mapping of string and numeric values to LogLevel enum
            static const std::unordered_map<std::string, LogLevel> levelMap = {
                {"DEBUG", LogLevel::DEBUG},
                {"INFO", LogLevel::INFO},
                {"WARNING", LogLevel::WARN},
                {"ERROR", LogLevel::ERR},
                
                // Numeric representations
                {"0", LogLevel::DEBUG},
                {"1", LogLevel::INFO},
                {"2", LogLevel::WARN},
                {"3", LogLevel::ERR},
            };

            // Look up the level in our mapping
            auto it = levelMap.find(levelStr);
            if (it != levelMap.end()) {
                std::lock_guard<std::mutex> lock(levelMutex);
                currentLevel = it->second;
            }
            break;  // Stop at first LOG_LEVEL= line found
        }
    }
}
