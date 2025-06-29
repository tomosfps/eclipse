/**
 * @file Macros.h
 * @brief Eclipse Logging Library - Convenience macros for logging
 * @author tomosfps
 * @date 2025
 * @version 2.0.0
 */

#pragma once

#include "Logger.h"
#include <sstream>
#include <vector>
#include <string>

/**
 * @brief Cross-platform function name macro
 *
 * Provides the function signature or name depending on the compiler.
 * - Clang/GCC: Uses __PRETTY_FUNCTION__ for detailed function signatures
 * - MSVC: Uses __FUNCSIG__ for function signatures
 * - Other: Falls back to __FUNCTION__ for basic function names
 */
#if defined(__clang__) || defined(__GNUC__)
#define ECLIPSE_FUNC_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define ECLIPSE_FUNC_NAME __FUNCSIG__
#else
#define ECLIPSE_FUNC_NAME __FUNCTION__
#endif

/**
 * @brief Macro to generate trace information with file, line, and function details
 *
 * Creates a formatted string containing the current file name (without path),
 * line number, and function name. This macro uses a lambda function to capture
 * the current location information at compile time.
 *
 * @return std::string Formatted trace string in the format "at filename:line [function]"
 *
 * @note The lambda captures __FILE__, __LINE__, and ECLIPSE_FUNC_NAME at the call site
 */
#define ETRACE_INFO() ([](const char *file, int line, const char *func) { \
    std::ostringstream oss; \
    std::string filename = file; \
    size_t lastSep = filename.find_last_of("\\/"); \
    if (lastSep != std::string::npos) filename = filename.substr(lastSep + 1); \
    oss << filename << ":" << line << " [" << func << "]"; \
    return oss.str(); }(__FILE__, __LINE__, ECLIPSE_FUNC_NAME))

/**
 * @brief Internal implementation function for logging macros
 *
 * This inline function serves as the common implementation for all logging macros.
 * It forwards the call to the Logger singleton with the appropriate parameters.
 *
 * @param tag Category or tag for the log message
 * @param msg The main log message
 * @param details Vector of additional detail strings
 * @param trace Trace information (file, line, function)
 * @param level The logging level for this message
 */
inline void ECLIPSE_MACRO_IMPL(const std::string &tag, const std::string &msg,
                               const std::vector<std::string> &details, const std::string &trace, Eclipse::ELevel level)
{
    Eclipse::Logger::getInstance().log(level, tag, msg, details, trace);
}

/**
 * @brief Internal implementation function for assertion macros
 *
 * This inline function serves as the implementation for assertion macros.
 * It forwards the call to the Logger's assert method.
 *
 * @param condition The boolean condition to test
 * @param tag Category or tag for the assertion
 * @param msg The error message to log if the assertion fails
 * @param details Vector of additional detail strings
 * @param trace Trace information (file, line, function)
 */
inline void ECLIPSE_ASSERT_IMPL(bool condition, const std::string &tag, const std::string &msg,
                                const std::vector<std::string> &details, const std::string &trace)
{
    Eclipse::Logger::getInstance().assert(condition, tag, msg, details, trace);
}

/**
 * @brief Log a debug message with automatic trace information
 *
 * Convenience macro for logging debug-level messages. Automatically captures
 * file, line, and function information for the trace.
 *
 * @param tag Category or tag for the message (e.g., "Database", "Network")
 * @param msg The main debug message
 * @param ... Optional additional details (converted to string)
 *
 * Example usage:
 * @code
 * ECLIPSE_DEBUG("Database", "Connection established", "host=localhost", "port=5432");
 * @endcode
 */
#define ECLIPSE_DEBUG(tag, msg, ...) \
    ECLIPSE_MACRO_IMPL(tag, msg, Eclipse::eclipse_make_details(#__VA_ARGS__), ETRACE_INFO(), Eclipse::ELevel::ECLIPSE_DEBUG)

/**
 * @brief Log an informational message with automatic trace information
 *
 * Convenience macro for logging info-level messages. Automatically captures
 * file, line, and function information for the trace.
 *
 * @param tag Category or tag for the message
 * @param msg The main informational message
 * @param ... Optional additional details (converted to string)
 *
 * Example usage:
 * @code
 * ECLIPSE_INFO("System", "Application started", "version=2.0.0");
 * @endcode
 */
#define ECLIPSE_INFO(tag, msg, ...) \
    ECLIPSE_MACRO_IMPL(tag, msg, Eclipse::eclipse_make_details(#__VA_ARGS__), ETRACE_INFO(), Eclipse::ELevel::ECLIPSE_INFO)

/**
 * @brief Log a warning message with automatic trace information
 *
 * Convenience macro for logging warning-level messages. Automatically captures
 * file, line, and function information for the trace.
 *
 * @param tag Category or tag for the message
 * @param msg The main warning message
 * @param ... Optional additional details (converted to string)
 *
 * Example usage:
 * @code
 * ECLIPSE_WARNING("Memory", "High memory usage detected", "usage=85%");
 * @endcode
 */
#define ECLIPSE_WARNING(tag, msg, ...) \
    ECLIPSE_MACRO_IMPL(tag, msg, Eclipse::eclipse_make_details(#__VA_ARGS__), ETRACE_INFO(), Eclipse::ELevel::ECLIPSE_WARN)

/**
 * @brief Log an error message with automatic trace information
 *
 * Convenience macro for logging error-level messages. Automatically captures
 * file, line, and function information for the trace.
 *
 * @param tag Category or tag for the message
 * @param msg The main error message
 * @param ... Optional additional details (converted to string)
 *
 * Example usage:
 * @code
 * ECLIPSE_ERROR("FileIO", "Failed to open file", "path=/tmp/data.txt", "errno=2");
 * @endcode
 */
#define ECLIPSE_ERROR(tag, msg, ...) \
    ECLIPSE_MACRO_IMPL(tag, msg, Eclipse::eclipse_make_details(#__VA_ARGS__), ETRACE_INFO(), Eclipse::ELevel::ECLIPSE_ERROR)

/**
 * @brief Log a fatal error message with automatic trace information
 *
 * Convenience macro for logging fatal-level messages. Automatically captures
 * file, line, and function information for the trace.
 *
 * @param tag Category or tag for the message
 * @param msg The main fatal error message
 * @param ... Optional additional details (converted to string)
 *
 * Example usage:
 * @code
 * ECLIPSE_FATAL("System", "Critical system failure", "component=core", "action=shutdown");
 * @endcode
 */
#define ECLIPSE_FATAL(tag, msg, ...) \
    ECLIPSE_MACRO_IMPL(tag, msg, Eclipse::eclipse_make_details(#__VA_ARGS__), ETRACE_INFO(), Eclipse::ELevel::ECLIPSE_FATAL)

/**
 * @brief Assert a condition and log an error if it fails
 *
 * Convenience macro for runtime assertions with logging. If the condition is false,
 * logs an error message with the provided details and trace information.
 *
 * @param condition The boolean condition to test
 * @param tag Category or tag for the assertion
 * @param msg The error message to log if the assertion fails
 * @param ... Optional additional details (converted to string)
 *
 * Example usage:
 * @code
 * ECLIPSE_ASSERT(ptr != nullptr, "Memory", "Null pointer detected", "variable=ptr");
 * @endcode
 */
#define ECLIPSE_ASSERT(condition, tag, msg, ...) \
    ECLIPSE_ASSERT_IMPL(condition, tag, msg, Eclipse::eclipse_make_details(#__VA_ARGS__), ETRACE_INFO())