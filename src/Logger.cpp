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
 * @brief Constructor - Initialize logger with default DEBUG level
 * 
 * Private constructor implementing singleton pattern. Sets the initial
 * log level to DEBUG and attempts to load configuration from environment.
 */
Logger::Logger() : currentLevel(LogLevel::DEBUG) {
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
            out << white << padding << "┗ at " << dim 
                << cleanTrace.substr(atPos + 3) << reset << "\n";
        } else {
            out << white << padding << "┗ " << dim << cleanTrace << reset << "\n";
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
 * @brief Load log level configuration from configuration files
 * 
 * Attempts to read log level configuration from .env or .ini files in the
 * current working directory. Tries .env first, then .ini if .env is not found.
 * Falls back to DEBUG level if no configuration files are found or contain
 * valid LOG_LEVEL settings.
 * 
 * Supported formats:
 * - .env files: LOG_LEVEL=DEBUG, LOG_LEVEL=INFO, etc.
 * - .ini files: LOG_LEVEL=DEBUG under [logging] section or standalone
 * - Numeric values: LOG_LEVEL=0 (DEBUG), LOG_LEVEL=1 (INFO), etc.
 * 
 * @note This method is called automatically during Logger initialization
 * @note The level setting is applied with thread-safe mutex protection
 */
void Logger::loadEnvLogLevel() {
    // Try to load from .env file first
    if (parseEnvFile(".env")) {
        return;
    }
    
    // If .env not found or doesn't contain LOG_LEVEL, try .ini
    if (parseIniFile(".ini") || parseIniFile("config.ini") || parseIniFile("settings.ini")) {
        return;
    }
    
    // If no configuration found, explicitly set to DEBUG as fallback
    std::lock_guard<std::mutex> lock(levelMutex);
    currentLevel = LogLevel::DEBUG;
}

/**
 * @brief Load log level from a specific configuration file
 * 
 * Loads configuration from a specified .env or .ini file path.
 * Automatically detects file type based on extension and calls
 * the appropriate parser.
 * 
 * @param configPath Path to the configuration file (.env or .ini)
 * @return true if configuration was loaded successfully, false otherwise
 * 
 * @example
 * @code
 * Logger& logger = Logger::getInstance();
 * if (logger.loadConfigFromFile("/path/to/config.ini")) {
 *     // Configuration loaded successfully
 * }
 * @endcode
 */
bool Logger::loadConfigFromFile(const std::string& configPath) {
    if (configPath.empty()) {
        return false;
    }
    
    // Determine file type by extension
    size_t dotPos = configPath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return false;
    }
    
    std::string extension = configPath.substr(dotPos);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == ".env") {
        return parseEnvFile(configPath);
    } else if (extension == ".ini") {
        return parseIniFile(configPath);
    }
    
    return false;
}

/**
 * @brief Parse .env format configuration file
 * 
 * Parses a .env format file looking for LOG_LEVEL=value lines.
 * Supports both string and numeric log level values.
 * 
 * @param filePath Path to the .env file
 * @return true if LOG_LEVEL was found and parsed successfully, false otherwise
 */
bool Logger::parseEnvFile(const std::string& filePath) {
    std::ifstream envFile(filePath);
    if (!envFile.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(envFile, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        if (line.find("LOG_LEVEL=") == 0) {
            std::string levelStr = line.substr(10);
            
            LogLevel level;
            if (parseLogLevelString(levelStr, level)) {
                std::lock_guard<std::mutex> lock(levelMutex);
                currentLevel = level;
                return true;
            }
            break;  // Stop at first LOG_LEVEL= line found
        }
    }
    return false;
}

/**
 * @brief Parse .ini format configuration file
 * 
 * Parses a .ini format file looking for LOG_LEVEL under [logging] section
 * or as a standalone LOG_LEVEL=value line. Supports both sectioned and
 * flat .ini file formats.
 * 
 * @param filePath Path to the .ini file
 * @return true if LOG_LEVEL was found and parsed successfully, false otherwise
 */
bool Logger::parseIniFile(const std::string& filePath) {
    std::ifstream iniFile(filePath);
    if (!iniFile.is_open()) {
        return false;
    }
    
    std::string line;
    bool inLoggingSection = false;
    bool foundLogLevel = false;
    
    while (std::getline(iniFile, line)) {
        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }
        
        // Check for section headers
        if (line[0] == '[' && line.back() == ']') {
            std::string sectionName = line.substr(1, line.length() - 2);
            std::transform(sectionName.begin(), sectionName.end(), sectionName.begin(), ::tolower);
            inLoggingSection = (sectionName == "logging" || sectionName == "log" 
                             || sectionName == "debugging" || sectionName == "debug");
            continue;
        }
        
        // Look for LOG_LEVEL setting
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);
            
            // Remove whitespace
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n\"'") + 1);
            
            // Convert key to uppercase for case-insensitive matching
            std::transform(key.begin(), key.end(), key.begin(), ::toupper);
            
            // Accept LOG_LEVEL in [logging] section or as standalone
            if (key == "LOG_LEVEL" && (inLoggingSection || !foundLogLevel)) {
                LogLevel level;
                if (parseLogLevelString(value, level)) {
                    std::lock_guard<std::mutex> lock(levelMutex);
                    currentLevel = level;
                    return true;
                }
                foundLogLevel = true;
            }
        }
    }
    return false;
}

/**
 * @brief Convert string log level to LogLevel enum
 * 
 * Converts string representations (both text and numeric) to LogLevel enum.
 * Supports case-insensitive string matching and numeric values.
 * 
 * @param levelStr String representation of log level
 * @param outLevel Reference to store the converted LogLevel
 * @return true if conversion was successful, false otherwise
 * 
 * Supported formats:
 * - String: "DEBUG", "INFO", "WARNING", "ERROR" (case-insensitive)
 * - Numeric: "0" (DEBUG), "1" (INFO), "2" (WARN), "3" (ERROR)
 */
bool Logger::parseLogLevelString(const std::string& levelStr, LogLevel& outLevel) {
    if (levelStr.empty()) {
        return false;
    }
    
    // Remove leading and trailing whitespace and quotes
    std::string cleanLevelStr = levelStr;
    
    // Remove leading whitespace and quotes
    cleanLevelStr.erase(0, cleanLevelStr.find_first_not_of(" \t\r\n\"'"));
    
    // Remove trailing whitespace and quotes
    cleanLevelStr.erase(cleanLevelStr.find_last_not_of(" \t\r\n\"'") + 1);
    
    if (cleanLevelStr.empty()) {
        return false;
    }
    
    // Convert to uppercase for case-insensitive matching
    std::string upperLevelStr = cleanLevelStr;
    std::transform(upperLevelStr.begin(), upperLevelStr.end(), upperLevelStr.begin(), ::toupper);
    
    // Mapping of string and numeric values to LogLevel enum
    static const std::unordered_map<std::string, LogLevel> levelMap = {
        {"DEBUG", LogLevel::DEBUG},
        {"INFO", LogLevel::INFO},
        {"WARNING", LogLevel::WARN},
        {"WARN", LogLevel::WARN},
        {"ERROR", LogLevel::ERR},
        {"ERR", LogLevel::ERR},
        
        // Numeric representations
        {"0", LogLevel::DEBUG},
        {"1", LogLevel::INFO},
        {"2", LogLevel::WARN},
        {"3", LogLevel::ERR},
    };
    
    auto it = levelMap.find(upperLevelStr);
    if (it != levelMap.end()) {
        outLevel = it->second;
        return true;
    }
    
    return false;
}
