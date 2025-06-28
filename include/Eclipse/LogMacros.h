/**
 * @file LogMacros.h
 * @brief Eclipse Logging Library - Convenient logging macros with automatic trace information
 * @author tomosfps
 * @date 2025
 * @version 1.0.3
 *
 * This file provides convenient macros for logging that automatically capture
 * source location information (file, line, function) for debugging purposes.
 * These macros are the recommended way to use the Eclipse logging system.
 */

#pragma once
#include "Logger.h"
#include <sstream>

#if defined(__clang__) || defined(__GNUC__)
#define FUNC_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define FUNC_NAME __FUNCSIG__
#else
#define FUNC_NAME __FUNCTION__
#endif

/**
 * @brief Implementation function for debug logging
 *
 * This inline function provides the actual implementation for debug log macros.
 * It's designed to be called by macros that automatically provide trace information.
 *
 * @param tag Category tag for the log message
 * @param msg Main message content
 * @param details Additional details or context
 * @param trace Source location information (file:line [function])
 */
inline void LOG_DEBUG_IMPL(const std::string &tag, const std::string &msg,
                           const std::string &details, const std::string &trace)
{
    Logger::getInstance().debug(tag, msg, details, trace);
}

/**
 * @brief Implementation function for info logging
 *
 * This inline function provides the actual implementation for info log macros.
 * It's designed to be called by macros that automatically provide trace information.
 *
 * @param tag Category tag for the log message
 * @param msg Main message content
 * @param details Additional details or context
 * @param trace Source location information (file:line [function])
 */
inline void LOG_INFO_IMPL(const std::string &tag, const std::string &msg,
                          const std::string &details, const std::string &trace)
{
    Logger::getInstance().info(tag, msg, details, trace);
}

/**
 * @brief Implementation function for warning logging
 *
 * This inline function provides the actual implementation for warning log macros.
 * It's designed to be called by macros that automatically provide trace information.
 *
 * @param tag Category tag for the log message
 * @param msg Main message content
 * @param details Additional details or context
 * @param trace Source location information (file:line [function])
 */
inline void LOG_WARNING_IMPL(const std::string &tag, const std::string &msg,
                             const std::string &details, const std::string &trace)
{
    Logger::getInstance().warning(tag, msg, details, trace);
}

/**
 * @brief Implementation function for error logging
 *
 * This inline function provides the actual implementation for error log macros.
 * It's designed to be called by macros that automatically provide trace information.
 *
 * @param tag Category tag for the log message
 * @param msg Main message content
 * @param details Additional details or context
 * @param trace Source location information (file:line [function])
 */
inline void LOG_ERROR_IMPL(const std::string &tag, const std::string &msg,
                           const std::string &details, const std::string &trace)
{
    Logger::getInstance().error(tag, msg, details, trace);
}

/**
 * @brief Macro to generate trace information automatically
 *
 * This macro captures the current source location including filename, line number,
 * and function name. It uses compiler-provided macros (__FILE__, __LINE__, __FUNCTION__)
 * to generate a formatted trace string.
 *
 * @return Formatted string: "at filename:line [function]"
 *
 * @note This macro uses a lambda expression to construct the string at the call site,
 *       ensuring accurate location information.
 */
#define TRACE_INFO() ([](const char *file, int line, const char *func) { \
    std::ostringstream oss; \
    std::string filename = file; \
    size_t lastSep = filename.find_last_of("\\/"); \
    if (lastSep != std::string::npos) filename = filename.substr(lastSep + 1); \
    oss << "at " << filename << ":" << line << " [" << func << "]"; \
    return oss.str(); }(__FILE__, __LINE__, FUNC_NAME))

///@{
/**
 * @name Debug Logging Macros
 * @brief Macros for debug-level logging with automatic trace information
 */

/**
 * @brief Log a debug message with automatic trace information
 *
 * @param tag Category tag for the message (e.g., "HTTP", "DB", "AUTH")
 * @param msg The main log message content
 *
 * @example
 * @code
 * LOG_DEBUG("NETWORK", "Sending HTTP request to server");
 * @endcode
 */
#define LOG_DEBUG(tag, msg) LOG_DEBUG_IMPL(tag, msg, "", TRACE_INFO())

/**
 * @brief Log a debug message with additional details and automatic trace information
 *
 * @param tag Category tag for the message
 * @param msg The main log message content
 * @param details Additional context or details
 *
 * @example
 * @code
 * LOG_DEBUG_DETAILS("HTTP", "Request failed", "Status: 404 Not Found");
 * @endcode
 */
#define LOG_DEBUG_DETAILS(tag, msg, details) LOG_DEBUG_IMPL(tag, msg, details, TRACE_INFO())
///@}

///@{
/**
 * @name Info Logging Macros
 * @brief Macros for info-level logging with automatic trace information
 */

/**
 * @brief Log an info message with automatic trace information
 *
 * @param tag Category tag for the message (e.g., "APP", "CONFIG", "STARTUP")
 * @param msg The main log message content
 *
 * @example
 * @code
 * LOG_INFO("APP", "Application started successfully");
 * @endcode
 */
#define LOG_INFO(tag, msg) LOG_INFO_IMPL(tag, msg, "", TRACE_INFO())

/**
 * @brief Log an info message with additional details and automatic trace information
 *
 * @param tag Category tag for the message
 * @param msg The main log message content
 * @param details Additional context or details
 *
 * @example
 * @code
 * LOG_INFO_DETAILS("CONFIG", "Configuration loaded", "Source: config.json");
 * @endcode
 */
#define LOG_INFO_DETAILS(tag, msg, details) LOG_INFO_IMPL(tag, msg, details, TRACE_INFO())
///@}

///@{
/**
 * @name Warning Logging Macros
 * @brief Macros for warning-level logging with automatic trace information
 */

/**
 * @brief Log a warning message with automatic trace information
 *
 * @param tag Category tag for the message (e.g., "CONFIG", "DEPRECATED", "PERF")
 * @param msg The main log message content
 *
 * @example
 * @code
 * LOG_WARNING("CONFIG", "Using default configuration");
 * @endcode
 */
#define LOG_WARNING(tag, msg) LOG_WARNING_IMPL(tag, msg, "", TRACE_INFO())

/**
 * @brief Log a warning message with additional details and automatic trace information
 *
 * @param tag Category tag for the message
 * @param msg The main log message content
 * @param details Additional context or details
 *
 * @example
 * @code
 * LOG_WARNING_DETAILS("PERF", "Slow query detected", "Duration: 5.2s");
 * @endcode
 */
#define LOG_WARNING_DETAILS(tag, msg, details) LOG_WARNING_IMPL(tag, msg, details, TRACE_INFO())
///@}

///@{
/**
 * @name Error Logging Macros
 * @brief Macros for error-level logging with automatic trace information
 */

/**
 * @brief Log an error message with automatic trace information
 *
 * @param tag Category tag for the message (e.g., "DB", "AUTH", "NETWORK")
 * @param msg The main log message content
 *
 * @example
 * @code
 * LOG_ERROR("DB", "Failed to connect to database");
 * @endcode
 */
#define LOG_ERROR(tag, msg) LOG_ERROR_IMPL(tag, msg, "", TRACE_INFO())

/**
 * @brief Log an error message with additional details and automatic trace information
 *
 * @param tag Category tag for the message
 * @param msg The main log message content
 * @param details Additional context or details
 *
 * @example
 * @code
 * LOG_ERROR_DETAILS("AUTH", "Authentication failed", "Invalid credentials");
 * @endcode
 */
#define LOG_ERROR_DETAILS(tag, msg, details) LOG_ERROR_IMPL(tag, msg, details, TRACE_INFO())
///@}
