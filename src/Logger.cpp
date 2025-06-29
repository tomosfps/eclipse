#include "Eclipse/Logger.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Eclipse
{
    Logger::Logger() : currentLevel(ELevel::ECLIPSE_DEBUG)
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
    }

    Logger &Logger::getInstance()
    {
        static std::once_flag flag;
        static Logger *instance = nullptr;
        std::call_once(flag, []()
                       { instance = new Logger(); });
        return *instance;
    }

    void Logger::setLevel(ELevel level)
    {
        std::lock_guard<std::mutex> lock(levelMutex);
        currentLevel = level;
    }

    ELevel Logger::getLevel() const
    {
        std::lock_guard<std::mutex> lock(levelMutex);
        return currentLevel;
    }

    void Logger::setLogFile(const std::string &filePath)
    {
        std::lock_guard<std::mutex> lock(fileMutex);
        logFilePath = filePath;
        if (logFileStream.is_open())
        {
            logFileStream.close();
        }
        logFileStream.open(logFilePath, std::ios::app);
    }

    void Logger::closeLogFile()
    {
        std::lock_guard<std::mutex> lock(fileMutex);
        if (logFileStream.is_open())
        {
            logFileStream.close();
        }
        logFilePath.clear();
    }

    void Logger::setOutputDestination(EOutput output)
    {
        std::lock_guard<std::mutex> lock(fileMutex);
        outputDestination = output;
    }

    EOutput Logger::getOutputDestination() const
    {
        std::lock_guard<std::mutex> lock(fileMutex);
        return outputDestination;
    }

    std::string Logger::getColour(ELevel level) const
    {
        switch (level)
        {
        case ELevel::ECLIPSE_DEBUG:
            return "\033[36m"; // Cyan
        case ELevel::ECLIPSE_INFO:
            return "\033[32m"; // Green
        case ELevel::ECLIPSE_WARN:
            return "\033[33m"; // Yellow
        case ELevel::ECLIPSE_ERROR:
            return "\033[31m"; // Red
        case ELevel::ECLIPSE_FATAL:
            return "\033[35m"; // Magenta
        default:
            return "\033[0m"; // Reset/Default
        }
    }

    std::string Logger::getLevelName(ELevel level) const
    {
        switch (level)
        {
        case ELevel::ECLIPSE_DEBUG:
            return "DEBUG";
        case ELevel::ECLIPSE_INFO:
            return "INFO";
        case ELevel::ECLIPSE_WARN:
            return "WARN";
        case ELevel::ECLIPSE_ERROR:
            return "ERROR";
        case ELevel::ECLIPSE_FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
        }
    }

    bool Logger::loadConfig(const std::string &configPath)
    {
        std::ifstream configFile(configPath);
        if (!configFile.is_open())
        {
            return false;
        }

        std::string line;
        while (std::getline(configFile, line))
        {
            std::istringstream iss(line);
            std::string key, value;
            if (std::getline(iss, key, '=') && std::getline(iss, value))
            {
                if (key == "ECLIPSE_LOG_LEVEL")
                {
                    ELevel level;
                    if (parseLevel(value, level))
                    {
                        std::lock_guard<std::mutex> lock(levelMutex);
                        currentLevel = level;
                    }
                }
            }
        }
        return true;
    }

    bool Logger::parseLevel(const std::string &value, ELevel &outLevel) const
    {
        std::string cleanValue = value;
        cleanValue.erase(0, cleanValue.find_first_not_of(" \t\r\n\"'"));
        cleanValue.erase(cleanValue.find_last_not_of(" \t\r\n\"'") + 1);

        if (cleanValue.empty())
        {
            return false;
        }

        std::transform(cleanValue.begin(), cleanValue.end(), cleanValue.begin(), ::toupper);

        static const std::unordered_map<std::string, ELevel> levelMap = {
            {"DEBUG", ELevel::ECLIPSE_DEBUG},
            {"INFO", ELevel::ECLIPSE_INFO},
            {"WARNING", ELevel::ECLIPSE_WARN},
            {"WARN", ELevel::ECLIPSE_WARN},
            {"ERROR", ELevel::ECLIPSE_ERROR},
            {"ERR", ELevel::ECLIPSE_ERROR},
            {"FATAL", ELevel::ECLIPSE_FATAL},
            {"0", ELevel::ECLIPSE_DEBUG},
            {"1", ELevel::ECLIPSE_INFO},
            {"2", ELevel::ECLIPSE_WARN},
            {"3", ELevel::ECLIPSE_ERROR},
            {"4", ELevel::ECLIPSE_FATAL}};

        auto it = levelMap.find(cleanValue);
        if (it != levelMap.end())
        {
            outLevel = it->second;
            return true;
        }
        return false;
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

    std::string Logger::truncatePath(const std::string &path) const
    {
        size_t lastSep = path.find_last_of("\\/");
        if (lastSep != std::string::npos)
        {
            return path.substr(lastSep + 1);
        }
        return path;
    }

    std::vector<std::string> eclipse_make_details(const std::string &details)
    {
        std::vector<std::string> result;
        std::istringstream iss(details);
        std::string detail;
        while (std::getline(iss, detail, ','))
        {
            detail.erase(0, detail.find_first_not_of(" \t\r\n\"'"));
            detail.erase(detail.find_last_not_of(" \t\r\n\"'") + 1);
            if (!detail.empty())
            {
                result.push_back(detail);
            }
        }
        return result;
    }

    void Logger::log(ELevel level, const std::string &tag, const std::string &msg,
                     const std::vector<std::string> &details, const std::string &trace)
    {
        if (level < currentLevel)
            return;

        std::lock_guard<std::mutex> lock(logMutex);

        std::ostringstream out;

        std::string grayColor = "\033[90m";
        std::string whiteColor = "\033[37m";
        std::string resetColor = "\033[0m";
        std::string boldColor = "\033[1m";
        std::string levelColor = getColour(level);

        std::string timestamp = getTimestamp();
        std::string levelName = getLevelName(level);

        const uint8_t maxLevelWidth = 5;
        std::string paddedLevelName = levelName;
        while (paddedLevelName.length() < maxLevelWidth)
        {
            paddedLevelName += " ";
        }

        size_t prefixLength = timestamp.length() + 3 + maxLevelWidth + 2;
        std::string indent(prefixLength, ' ');

        out << grayColor << "[" << timestamp << "] "
            << levelColor << boldColor << paddedLevelName << resetColor << ": "
            << whiteColor << "┏ "
            << whiteColor << "[" << levelColor << tag << whiteColor << "] "
            << whiteColor << msg << resetColor << "\n";

        if (!trace.empty())
        {
            out << indent << whiteColor << "┃ "
                << levelColor << "at: " << whiteColor << trace << resetColor << "\n";
        }

        if (!details.empty())
        {
            for (size_t i = 0; i < details.size(); ++i)
            {
                if (i == details.size() - 1)
                {
                    out << indent << whiteColor << "┗ "
                        << grayColor << "[" << (i + 1) << "] " << details[i] << resetColor << "\n";
                }
                else
                {
                    out << indent << whiteColor << "┃ "
                        << grayColor << "[" << (i + 1) << "] " << details[i] << resetColor << "\n";
                }
            }
        }
        else if (!trace.empty())
        {
            out << indent << whiteColor << "┗" << resetColor << "\n";
        }

        if (outputDestination == EOutput::CONSOLE || outputDestination == EOutput::BOTH)
        {
            std::cout << out.str();
        }

        if (outputDestination == EOutput::FILE || outputDestination == EOutput::BOTH)
        {
            std::lock_guard<std::mutex> fileLock(fileMutex);
            if (logFileStream.is_open())
            {
                std::string fileOutput = out.str();
                size_t pos = 0;
                while ((pos = fileOutput.find("\033[", pos)) != std::string::npos)
                {
                    size_t end = fileOutput.find("m", pos);
                    if (end != std::string::npos)
                    {
                        fileOutput.erase(pos, end - pos + 1);
                    }
                    else
                    {
                        break;
                    }
                }
                logFileStream << fileOutput;
                logFileStream.flush();
            }
        }
    }

    bool Logger::assert(bool condition, const std::string &tag, const std::string &msg,
                        const std::vector<std::string> &details, const std::string &trace)
    {
        if (!condition)
        {
            log(ELevel::ECLIPSE_FATAL, tag, msg, details, trace);
            return false;
        }
        return true;
    }
}
