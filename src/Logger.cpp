/**
 * @file Logger.cpp
 * @brief Eclipse Logging Library - Implementation of the Logger class
 * @author tomosfps
 * @date 2025
 * @version 1.0.3
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

Logger::Logger() : currentLevel(LogLevel::ECLIPSE_DEBUG)
{
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE)
    {
        DWORD mode;
        GetConsoleMode(hConsole, &mode);
        SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    loadEnvLogLevel();
}

Logger &Logger::getInstance()
{
    static std::once_flag flag;
    static Logger *instance = nullptr;
    std::call_once(flag, []()
                   { instance = new Logger(); });
    return *instance;
}

void Logger::setLogLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(levelMutex);
    currentLevel = level;
}

LogLevel Logger::getLogLevel() const
{
    std::lock_guard<std::mutex> lock(levelMutex);
    return currentLevel;
}

void Logger::debug(const std::string &tag, const std::string &msg, const std::string &details, const std::string &trace)
{
    logInternal(LogLevel::ECLIPSE_DEBUG, tag, msg, details, trace);
}

void Logger::info(const std::string &tag, const std::string &msg, const std::string &details, const std::string &trace)
{
    logInternal(LogLevel::ECLIPSE_INFO, tag, msg, details, trace);
}

void Logger::warning(const std::string &tag, const std::string &msg, const std::string &details, const std::string &trace)
{
    logInternal(LogLevel::ECLIPSE_WARN, tag, msg, details, trace);
}

void Logger::error(const std::string &tag, const std::string &msg, const std::string &details, const std::string &trace)
{
    logInternal(LogLevel::ECLIPSE_ERROR, tag, msg, details, trace);
}

void Logger::logInternal(LogLevel level, const std::string &tag, const std::string &message,
                         const std::string &details, const std::string &trace)
{
    // First check: level filtering with separate lock scope
    {
        std::lock_guard<std::mutex> levelLock(levelMutex);
        if (level < currentLevel)
            return;
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
    if (!trace.empty() && details.empty())
    {
        std::string cleanTrace = extractFilename(trace);
        size_t atPos = cleanTrace.find("at ");
        if (atPos != std::string::npos)
        {
            out << white << padding << "┗ at " << dim
                << cleanTrace.substr(atPos + 3) << reset << "\n";
        }
        else
        {
            out << white << padding << "┗ " << dim << cleanTrace << reset << "\n";
        }
    }
    else
    {
        if (!trace.empty())
        {
            std::string cleanTrace = extractFilename(trace);
            size_t atPos = cleanTrace.find("at ");
            if (atPos != std::string::npos)
            {
                out << white << padding << "┃ at " << dim
                    << cleanTrace.substr(atPos + 3) << reset << "\n";
            }
            else
            {
                out << white << padding << "┃ " << dim << cleanTrace << reset << "\n";
            }
        }
        if (!details.empty())
        {
            out << white << padding << "┗ [1] " << dim << details << reset << "\n";
        }
    }

    std::cout << out.str();
    std::cout.flush();
}

std::string Logger::extractFilename(const std::string &trace) const
{
    size_t atPos = trace.find("at ");
    if (atPos == std::string::npos)
        return trace;

    size_t pathStart = atPos + 3;
    size_t colonPos = trace.find(':', pathStart);
    if (colonPos == std::string::npos)
        return trace;
    std::string fullPath = trace.substr(pathStart, colonPos - pathStart);

    // Find last separator (\ or /)
    size_t lastSep = fullPath.find_last_of("\\/");
    std::string filename = (lastSep != std::string::npos) ? fullPath.substr(lastSep + 1) : fullPath;

    // Reconstruct the trace with just the filename
    return trace.substr(0, pathStart) + filename + trace.substr(colonPos);
}

std::string Logger::getColor(LogLevel level) const
{
    switch (level)
    {
    case LogLevel::ECLIPSE_DEBUG:
        return "\033[36m"; // Cyan
    case LogLevel::ECLIPSE_INFO:
        return "\033[32m"; // Green
    case LogLevel::ECLIPSE_WARN:
        return "\033[33m"; // Yellow
    case LogLevel::ECLIPSE_ERROR:
        return "\033[31m"; // Red
    default:
        return "\033[0m"; // Reset/Default
    }
}

std::string Logger::getLevelName(LogLevel level) const
{
    switch (level)
    {
    case LogLevel::ECLIPSE_DEBUG:
        return "debug";
    case LogLevel::ECLIPSE_INFO:
        return "info";
    case LogLevel::ECLIPSE_WARN:
        return "warn";
    case LogLevel::ECLIPSE_ERROR:
        return "error";
    default:
        return "unknown";
    }
}

std::string Logger::getTimestamp() const
{
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

void Logger::loadEnvLogLevel()
{
    // Try to load from .env file first
    if (parseEnvFile(".env"))
    {
        return;
    }

    // If .env not found or doesn't contain LOG_LEVEL, try .ini
    if (parseIniFile(".ini") || parseIniFile("config.ini") || parseIniFile("settings.ini"))
    {
        return;
    }

    // If no configuration found, explicitly set to ECLIPSE_DEBUG as fallback
    std::lock_guard<std::mutex> lock(levelMutex);
    currentLevel = LogLevel::ECLIPSE_DEBUG;
}

bool Logger::loadConfigFromFile(const std::string &configPath)
{
    if (configPath.empty())
    {
        return false;
    }

    // Determine file type by extension
    size_t dotPos = configPath.find_last_of('.');
    if (dotPos == std::string::npos)
    {
        return false;
    }

    std::string extension = configPath.substr(dotPos);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (extension == ".env")
    {
        return parseEnvFile(configPath);
    }
    else if (extension == ".ini")
    {
        return parseIniFile(configPath);
    }

    return false;
}

bool Logger::parseEnvFile(const std::string &filePath)
{
    std::ifstream envFile(filePath);
    if (!envFile.is_open())
    {
        return false;
    }

    std::string line;
    while (std::getline(envFile, line))
    {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        if (line.find("LOG_LEVEL=") == 0)
        {
            std::string levelStr = line.substr(10);

            LogLevel level;
            if (parseLogLevelString(levelStr, level))
            {
                std::lock_guard<std::mutex> lock(levelMutex);
                currentLevel = level;
                return true;
            }
            break; // Stop at first LOG_LEVEL= line found
        }
    }
    return false;
}

bool Logger::parseIniFile(const std::string &filePath)
{
    std::ifstream iniFile(filePath);
    if (!iniFile.is_open())
    {
        return false;
    }

    std::string line;
    bool inLoggingSection = false;
    bool foundLogLevel = false;

    while (std::getline(iniFile, line))
    {
        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#')
        {
            continue;
        }

        // Check for section headers
        if (line[0] == '[' && line.back() == ']')
        {
            std::string sectionName = line.substr(1, line.length() - 2);
            std::transform(sectionName.begin(), sectionName.end(), sectionName.begin(), ::tolower);
            inLoggingSection = (sectionName == "logging" || sectionName == "log" || sectionName == "debugging" || sectionName == "debug");
            continue;
        }

        // Look for LOG_LEVEL setting
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos)
        {
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);

            // Remove whitespace
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n\"'") + 1);

            // Convert key to uppercase for case-insensitive matching
            std::transform(key.begin(), key.end(), key.begin(), ::toupper);

            // Accept LOG_LEVEL in [logging] section or as standalone
            if (key == "LOG_LEVEL" && (inLoggingSection || !foundLogLevel))
            {
                LogLevel level;
                if (parseLogLevelString(value, level))
                {
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

std::string Logger::getLogLevelName() const
{
    std::lock_guard<std::mutex> lock(levelMutex);
    return getLevelName(currentLevel);
}

bool Logger::parseLogLevelString(const std::string &levelStr, LogLevel &outLevel)
{
    if (levelStr.empty())
    {
        return false;
    }

    std::string cleanLevelStr = levelStr;
    cleanLevelStr.erase(0, cleanLevelStr.find_first_not_of(" \t\r\n\"'"));
    cleanLevelStr.erase(cleanLevelStr.find_last_not_of(" \t\r\n\"'") + 1);

    if (cleanLevelStr.empty())
    {
        return false;
    }

    std::string upperLevelStr = cleanLevelStr;
    std::transform(upperLevelStr.begin(), upperLevelStr.end(), upperLevelStr.begin(), ::toupper);

    static const std::unordered_map<std::string, LogLevel> levelMap = {
        {"DEBUG", LogLevel::ECLIPSE_DEBUG},
        {"INFO", LogLevel::ECLIPSE_INFO},
        {"WARNING", LogLevel::ECLIPSE_WARN},
        {"WARN", LogLevel::ECLIPSE_WARN},
        {"ERROR", LogLevel::ECLIPSE_ERROR},
        {"ERR", LogLevel::ECLIPSE_ERROR},

        // Numeric representations
        {"0", LogLevel::ECLIPSE_DEBUG},
        {"1", LogLevel::ECLIPSE_INFO},
        {"2", LogLevel::ECLIPSE_WARN},
        {"3", LogLevel::ECLIPSE_ERROR},
    };

    auto it = levelMap.find(upperLevelStr);
    if (it != levelMap.end())
    {
        outLevel = it->second;
        return true;
    }

    return false;
}
