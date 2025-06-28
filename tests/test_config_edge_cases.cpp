#include "Eclipse/Logger.h"
#include "Eclipse/LogMacros.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

// Test configuration file parsing edge cases
class ConfigTestRunner
{
private:
    int totalTests = 0;
    int passedTests = 0;
    std::ostringstream output;

public:
    void test(const std::string &testName, bool condition)
    {
        totalTests++;
        output << "[CONFIG_TEST] " << testName << ": ";
        if (condition)
        {
            output << "PASSED\n";
            passedTests++;
        }
        else
        {
            output << "FAILED\n";
        }
    }

    bool runAll()
    {
        std::cout << output.str();
        std::cout << "\n=== Config Test Summary ===\n";
        std::cout << "Passed: " << passedTests << "/" << totalTests << std::endl;

        if (passedTests == totalTests)
        {
            std::cout << "All config tests PASSED!\n"
                      << std::endl;
            return true;
        }
        else
        {
            std::cout << "Some config tests FAILED!\n"
                      << std::endl;
            return false;
        }
    }
};

void testEdgeCases(ConfigTestRunner &runner)
{
    Logger &logger = Logger::getInstance();

    // Test .env with comments and whitespace
    {
        std::ofstream file("test_comments.env");
        file << "# This is a comment\n";
        file << "   \n";               // Empty line with spaces
        file << "LOG_LEVEL=INFO   \n"; // Trailing spaces
        file << "# Another comment\n";
        file.close();

        bool loaded = logger.loadConfigFromFile("test_comments.env");
        runner.test("Parse .env with comments and whitespace", loaded && logger.getLogLevel() == LogLevel::ECLIPSE_INFO);
        std::remove("test_comments.env");
    }

    // Test .ini with multiple sections
    {
        std::ofstream file("test_sections.ini");
        file << "[database]\n";
        file << "host=localhost\n";
        file << "[logging]\n";
        file << "LOG_LEVEL=ERROR\n";
        file << "[application]\n";
        file << "name=test\n";
        file.close();

        bool loaded = logger.loadConfigFromFile("test_sections.ini");
        runner.test("Parse .ini with multiple sections", loaded && logger.getLogLevel() == LogLevel::ECLIPSE_ERROR);
        std::remove("test_sections.ini");
    }

    // Test case insensitive log levels
    {
        std::ofstream file("test_case.env");
        file << "LOG_LEVEL=debug\n"; // lowercase
        file.close();

        bool loaded = logger.loadConfigFromFile("test_case.env");
        runner.test("Case insensitive log level parsing", loaded && logger.getLogLevel() == LogLevel::ECLIPSE_DEBUG);
        std::remove("test_case.env");
    }

    // Test quoted values
    {
        std::ofstream file("test_quotes.env");
        file << "LOG_LEVEL=\"WARNING\"\n";
        file.close();

        bool loaded = logger.loadConfigFromFile("test_quotes.env");
        runner.test("Parse quoted log level values", loaded && logger.getLogLevel() == LogLevel::ECLIPSE_WARN);
        std::remove("test_quotes.env");
    }

    // Test invalid log level
    {
        std::ofstream file("test_invalid.env");
        file << "LOG_LEVEL=INVALID\n";
        file.close();

        LogLevel beforeLevel = logger.getLogLevel();
        bool loaded = logger.loadConfigFromFile("test_invalid.env");
        LogLevel afterLevel = logger.getLogLevel();

        runner.test("Handle invalid log level gracefully", !loaded || beforeLevel == afterLevel);
        std::remove("test_invalid.env");
    }

    // Test empty LOG_LEVEL value
    {
        std::ofstream file("test_empty.env");
        file << "LOG_LEVEL=\n";
        file.close();

        LogLevel beforeLevel = logger.getLogLevel();
        bool loaded = logger.loadConfigFromFile("test_empty.env");
        LogLevel afterLevel = logger.getLogLevel();

        runner.test("Handle empty LOG_LEVEL value", !loaded || beforeLevel == afterLevel);
        std::remove("test_empty.env");
    }
}

int main()
{
    std::cout << "=== Eclipse Logger Configuration Edge Cases Test ===\n"
              << std::endl;

    ConfigTestRunner runner;

    try
    {
        testEdgeCases(runner);

        bool allPassed = runner.runAll();
        return allPassed ? 0 : 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Config test execution failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Config test execution failed with unknown exception" << std::endl;
        return 1;
    }
}
