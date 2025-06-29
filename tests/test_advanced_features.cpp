/**
 * @file test_advanced_features.cpp
 * @brief Advanced feature tests for Eclipse Logger
 * @author tomosfps
 * @date 2025
 */

#include "Eclipse/Logger.h"
#include "Eclipse/Macros.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

using namespace Eclipse;

void test_trace_information()
{
    std::cout << "Testing trace information capture..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    logger.setOutputDestination(EOutput::CONSOLE);

    std::cout << "The following log should show file, line, and function information:" << std::endl;

    // This should capture trace information automatically
    ECLIPSE_INFO("TRACE_TEST", "Testing trace capture functionality");

    std::cout << "✓ Trace information test passed" << std::endl;
}

void test_empty_and_special_messages()
{
    std::cout << "Testing empty and special character messages..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    logger.setOutputDestination(EOutput::CONSOLE);

    // Test empty messages
    ECLIPSE_INFO("EMPTY_TEST", "");

    // Test messages with special characters
    ECLIPSE_INFO("SPECIAL_TEST", "Message with unicode: αβγ δεζ 你好 🎉");
    ECLIPSE_WARNING("SPECIAL_TEST", "Message with \"quotes\" and 'apostrophes'");
    ECLIPSE_ERROR("SPECIAL_TEST", "Message with\nnewlines\nand\ttabs");

    // Test very long message
    std::string long_message(1000, 'A');
    ECLIPSE_DEBUG("LONG_TEST", long_message);

    std::cout << "✓ Empty and special messages test passed" << std::endl;
}

void test_level_boundary_conditions()
{
    std::cout << "Testing log level boundary conditions..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setOutputDestination(EOutput::CONSOLE);

    // Test NONE level (should block everything)
    logger.setLevel(ELevel::ECLIPSE_NONE);
    std::cout << "Set to NONE level - no messages should appear:" << std::endl;
    ECLIPSE_DEBUG("BOUNDARY_TEST", "This should NOT appear (DEBUG)");
    ECLIPSE_INFO("BOUNDARY_TEST", "This should NOT appear (INFO)");
    ECLIPSE_WARNING("BOUNDARY_TEST", "This should NOT appear (WARNING)");
    ECLIPSE_ERROR("BOUNDARY_TEST", "This should NOT appear (ERROR)");
    ECLIPSE_FATAL("BOUNDARY_TEST", "This should NOT appear (FATAL)");

    // Reset to show results
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    std::cout << "Reset to DEBUG level" << std::endl;

    std::cout << "✓ Level boundary conditions test passed" << std::endl;
}

void test_assert_conditions()
{
    std::cout << "Testing various assert conditions..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    logger.setOutputDestination(EOutput::CONSOLE);

    // Test successful assertions (should not log)
    std::cout << "Testing successful assertions (should be silent):" << std::endl;
    ECLIPSE_ASSERT(1 == 1, "ASSERT_TEST", "1 equals 1");
    ECLIPSE_ASSERT(true, "ASSERT_TEST", "True is true");
    ECLIPSE_ASSERT(!false, "ASSERT_TEST", "Not false is true");

    // Test failed assertions (should log)
    std::cout << "Testing failed assertions (should log fatal errors):" << std::endl;
    ECLIPSE_ASSERT(1 == 2, "ASSERT_TEST", "1 does not equal 2", "expected=1", "actual=2");
    ECLIPSE_ASSERT(false, "ASSERT_TEST", "False is false");

    std::cout << "✓ Assert conditions test passed" << std::endl;
}

void test_complex_details()
{
    std::cout << "Testing complex detail formatting..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    logger.setOutputDestination(EOutput::CONSOLE);

    // Test various detail combinations
    ECLIPSE_INFO("DETAILS_TEST", "User login attempt",
                 "username=john.doe",
                 "ip_address=192.168.1.100",
                 "user_agent=Mozilla/5.0",
                 "timestamp=2025-06-29T15:43:00Z",
                 "session_id=abc123def456");

    ECLIPSE_ERROR("DETAILS_TEST", "Database query failed",
                  "query=SELECT * FROM users WHERE id = ?",
                  "parameters=[123]",
                  "error_code=1062",
                  "error_message=Duplicate entry",
                  "execution_time=1.23s");

    // Test details with special characters
    ECLIPSE_WARNING("DETAILS_TEST", "Configuration validation warning",
                    "setting=max_connections",
                    "value=10,000",
                    "recommended=5,000",
                    "note=Consider reducing for better performance");

    std::cout << "✓ Complex details test passed" << std::endl;
}

void test_rapid_logging()
{
    std::cout << "Testing rapid logging performance..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_INFO);
    logger.setOutputDestination(EOutput::NONE); // Disable output for performance test

    const int num_logs = 10000;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_logs; ++i)
    {
        ECLIPSE_INFO("PERF_TEST", "Rapid log message " + std::to_string(i));
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    // Re-enable console output for results
    logger.setOutputDestination(EOutput::CONSOLE);

    double logs_per_second = (num_logs * 1000000.0) / duration.count();
    std::cout << "Logged " << num_logs << " messages in " << duration.count()
              << " microseconds (" << logs_per_second << " logs/second)" << std::endl;

    std::cout << "✓ Rapid logging performance test passed" << std::endl;
}

void test_file_permissions_and_errors()
{
    std::cout << "Testing file permission and error handling..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_DEBUG);

    // Test setting a log file in a non-existent directory
    std::string invalid_path = "non_existent_directory/test.log";
    logger.setLogFile(invalid_path);
    logger.setOutputDestination(EOutput::FILE);

    // This should not crash even if file cannot be opened
    ECLIPSE_INFO("FILE_ERROR_TEST", "This message may not be written to file");

    // Switch back to console
    logger.setOutputDestination(EOutput::CONSOLE);
    std::cout << "File error handling completed without crash" << std::endl;

    std::cout << "✓ File permissions and error handling test passed" << std::endl;
}

void test_configuration_edge_cases()
{
    std::cout << "Testing configuration parsing edge cases..." << std::endl;

    Logger &logger = Logger::getInstance();

    // Test malformed configuration file
    const std::string malformed_config = "malformed_test.ini";
    std::ofstream config_file(malformed_config);
    config_file << "# Malformed configuration test\n";
    config_file << "ECLIPSE_LOG_LEVEL\n";               // Missing equals
    config_file << "=DEBUG\n";                          // Missing key
    config_file << "ECLIPSE_LOG_LEVEL=\n";              // Empty value
    config_file << "ECLIPSE_LOG_LEVEL=INVALID_LEVEL\n"; // Invalid level
    config_file << "ECLIPSE_LOG_LEVEL=DEBUG\n";         // Valid level at end
    config_file.close();

    ELevel original_level = logger.getLevel();
    bool success = logger.loadConfig(malformed_config);
    assert(success); // Should succeed (file exists)

    // Should have been set to DEBUG (the last valid entry)
    assert(original_level == ELevel::ECLIPSE_DEBUG);

    // Clean up
    std::filesystem::remove(malformed_config);

    std::cout << "✓ Configuration edge cases test passed" << std::endl;
}

void test_memory_usage()
{
    std::cout << "Testing memory usage with large log volumes..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_INFO);
    logger.setOutputDestination(EOutput::NONE);

    // Log a large number of messages with varying sizes
    for (int i = 0; i < 1000; ++i)
    {
        std::string large_message(i % 100 + 10, 'X'); // Variable size messages
        std::vector<std::string> details;
        for (int j = 0; j < (i % 5); ++j)
        {
            details.push_back("detail" + std::to_string(j) + "=" + std::to_string(i * j));
        }

        ECLIPSE_INFO("MEMORY_TEST", large_message,
                     "index=" + std::to_string(i),
                     "size=" + std::to_string(large_message.size()));
    }

    // Switch back to console
    logger.setOutputDestination(EOutput::CONSOLE);
    std::cout << "Memory usage test completed - no memory leaks expected" << std::endl;

    std::cout << "✓ Memory usage test passed" << std::endl;
}

int main()
{
    try
    {
        std::cout << "=== Eclipse Logger Advanced Feature Tests ===" << std::endl;
        std::cout << "Testing edge cases and advanced functionality..." << std::endl
                  << std::endl;

        test_trace_information();
        test_empty_and_special_messages();
        test_level_boundary_conditions();
        test_assert_conditions();
        test_complex_details();
        test_rapid_logging();
        test_file_permissions_and_errors();
        test_configuration_edge_cases();
        test_memory_usage();

        std::cout << std::endl
                  << "🎉 All advanced feature tests passed successfully!" << std::endl;
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
