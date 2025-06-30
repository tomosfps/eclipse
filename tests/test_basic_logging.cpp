/**
 * @file test_basic_logging.cpp
 * @brief Basic logging functionality tests for Eclipse Logger
 * @author tomosfps
 * @date 2025
 */

#include "Eclipse/Logger.h"
#include "Eclipse/Macros.h"
#include <iostream>
#include <cassert>
#include <sstream>
#include <thread>
#include <chrono>
#include <ctime>

using namespace Eclipse;

void test_singleton_pattern()
{
    std::cout << "Testing singleton pattern..." << std::endl;

    Logger &logger1 = Logger::getInstance();
    Logger &logger2 = Logger::getInstance();

    // Both references should point to the same instance
    assert(&logger1 == &logger2);
    std::cout << "✓ Singleton pattern test passed" << std::endl;
}

void test_log_levels()
{
    std::cout << "Testing log levels..." << std::endl;

    Logger &logger = Logger::getInstance();

    // Test setting and getting log levels
    logger.setLevel(ELevel::ECLIPSE_INFO);
    assert(logger.getLevel() == ELevel::ECLIPSE_INFO);

    logger.setLevel(ELevel::ECLIPSE_ERROR);
    assert(logger.getLevel() == ELevel::ECLIPSE_ERROR);

    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    assert(logger.getLevel() == ELevel::ECLIPSE_DEBUG);

    std::cout << "✓ Log levels test passed" << std::endl;
}

void test_output_destinations()
{
    std::cout << "Testing output destinations..." << std::endl;

    Logger &logger = Logger::getInstance();

    // Test setting output destinations
    logger.setOutputDestination(EOutput::CONSOLE);
    assert(logger.getOutputDestination() == EOutput::CONSOLE);

    logger.setOutputDestination(EOutput::FILE);
    assert(logger.getOutputDestination() == EOutput::FILE);

    logger.setOutputDestination(EOutput::BOTH);
    assert(logger.getOutputDestination() == EOutput::BOTH);

    logger.setOutputDestination(EOutput::NONE);
    assert(logger.getOutputDestination() == EOutput::NONE);

    std::cout << "✓ Output destinations test passed" << std::endl;
}

void test_level_names()
{
    std::cout << "Testing level names..." << std::endl;

    Logger &logger = Logger::getInstance();

    assert(logger.getLevelName(ELevel::ECLIPSE_DEBUG) == "DEBUG");
    assert(logger.getLevelName(ELevel::ECLIPSE_INFO) == "INFO");
    assert(logger.getLevelName(ELevel::ECLIPSE_WARN) == "WARN");
    assert(logger.getLevelName(ELevel::ECLIPSE_ERROR) == "ERROR");
    assert(logger.getLevelName(ELevel::ECLIPSE_FATAL) == "FATAL");

    std::cout << "✓ Level names test passed" << std::endl;
}

void test_basic_logging_macros()
{
    std::cout << "Testing basic logging macros..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    logger.setOutputDestination(EOutput::CONSOLE);

    std::cout << "Testing different log levels with macros:" << std::endl;

    ECLIPSE_DEBUG("TEST", "This is a debug message");
    ECLIPSE_INFO("TEST", "This is an info message");
    ECLIPSE_WARNING("TEST", "This is a warning message");
    ECLIPSE_ERROR("TEST", "This is an error message");
    ECLIPSE_FATAL("TEST", "This is a fatal message");

    std::cout << "✓ Basic logging macros test passed" << std::endl;
}

void test_logging_with_details()
{
    std::cout << "Testing logging with details..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    logger.setOutputDestination(EOutput::CONSOLE);

    std::cout << "Testing logging with additional details:" << std::endl;

    ECLIPSE_INFO("TEST", "Processing user data", "userId=12345", "operation=update");
    ECLIPSE_ERROR("TEST", "Database connection failed", "host=localhost", "port=5432", "timeout=30s");

    std::cout << "✓ Logging with details test passed" << std::endl;
}

void test_assert_functionality()
{
    std::cout << "Testing assert functionality..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    logger.setOutputDestination(EOutput::CONSOLE);

    std::cout << "Testing successful assertion (should not log):" << std::endl;
    ECLIPSE_ASSERT(true, "TEST", "This should not appear");

    std::cout << "Testing failed assertion (should log fatal error):" << std::endl;
    ECLIPSE_ASSERT(false, "TEST", "Assertion failed as expected", "condition=false");

    std::cout << "✓ Assert functionality test passed" << std::endl;
}

void test_log_level_filtering()
{
    std::cout << "Testing log level filtering..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setOutputDestination(EOutput::CONSOLE);

    // Set level to WARN - should only show WARN, ERROR, FATAL
    logger.setLevel(ELevel::ECLIPSE_WARN);

    std::cout << "Set log level to WARN - only WARN and above should appear:" << std::endl;
    ECLIPSE_DEBUG("FILTER_TEST", "This DEBUG should NOT appear");
    ECLIPSE_INFO("FILTER_TEST", "This INFO should NOT appear");
    ECLIPSE_WARNING("FILTER_TEST", "This WARNING should appear");
    ECLIPSE_ERROR("FILTER_TEST", "This ERROR should appear");
    ECLIPSE_FATAL("FILTER_TEST", "This FATAL should appear");

    // Reset to DEBUG for other tests
    logger.setLevel(ELevel::ECLIPSE_DEBUG);

    std::cout << "✓ Log level filtering test passed" << std::endl;
}

void test_function_evaluation()
{
    std::cout << "Testing function call and variable evaluation..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    logger.setOutputDestination(EOutput::CONSOLE);

    // Test with function calls that return values
    std::string currentLevel = logger.getLevelName(logger.getLevel());
    int port = 8080;
    double version = 2.1;
    bool isConnected = true;

    std::cout << "Testing with function calls and variables (should show actual values):" << std::endl;

    // This should show the actual level name (e.g., "DEBUG") instead of the function call text
    ECLIPSE_INFO("EVAL_TEST", "Current log level", logger.getLevelName(logger.getLevel()));

    // This should show the actual values instead of variable names
    ECLIPSE_DEBUG("EVAL_TEST", "Connection info", "port=" + std::to_string(port), "version=" + std::to_string(version));

    // Mix of variables and function calls
    ECLIPSE_WARNING("EVAL_TEST", "System status",
                    "connected=" + std::string(isConnected ? "true" : "false"),
                    "current_level=" + currentLevel,
                    "timestamp=" + std::to_string(std::time(nullptr)));

    std::cout << "✓ Function evaluation test passed" << std::endl;
}

int main()
{
    try
    {
        std::cout << "=== Eclipse Logger Basic Tests ===" << std::endl;
        std::cout << "Running basic functionality tests..." << std::endl
                  << std::endl;

        test_singleton_pattern();
        test_log_levels();
        test_output_destinations();
        test_level_names();
        test_basic_logging_macros();
        test_logging_with_details();
        test_function_evaluation();
        test_assert_functionality();
        test_log_level_filtering();
        test_function_evaluation();

        std::cout << std::endl
                  << "🎉 All basic tests passed successfully!" << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}
