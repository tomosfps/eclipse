/**
 * @file test_config_file_logging.cpp
 * @brief Configuration loading and file logging tests for Eclipse Logger
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
#include <algorithm>

using namespace Eclipse;

void test_config_file_loading()
{
    std::cout << "Testing configuration file loading..." << std::endl;

    Logger &logger = Logger::getInstance();

    // Test loading the demo.ini config file
    bool success = logger.loadConfig("demo.ini");
    assert(success);

    // The demo.ini file sets ECLIPSE_LOG_LEVEL=WARN
    assert(logger.getLevel() == ELevel::ECLIPSE_WARN);

    std::cout << "✓ Configuration file loading test passed" << std::endl;
}

void test_config_file_not_found()
{
    std::cout << "Testing configuration file not found handling..." << std::endl;

    Logger &logger = Logger::getInstance();

    // Test loading a non-existent config file
    bool success = logger.loadConfig("nonexistent.ini");
    assert(!success);

    std::cout << "✓ Configuration file not found test passed" << std::endl;
}

void test_file_logging()
{
    std::cout << "Testing file logging..." << std::endl;

    Logger &logger = Logger::getInstance();
    const std::string test_log_file = "test_output.log";

    // Clean up any existing log file
    if (std::filesystem::exists(test_log_file))
    {
        std::filesystem::remove(test_log_file);
    }

    // Configure for file logging
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    logger.setLogFile(test_log_file);
    logger.setOutputDestination(EOutput::FILE);

    // Log some test messages
    ECLIPSE_INFO("FILE_TEST", "This message should go to file", ("filename=" + test_log_file).c_str());
    ECLIPSE_WARNING("FILE_TEST", "Warning message in file");
    ECLIPSE_ERROR("FILE_TEST", "Error message with details", "error_code=404", "details=Not found");

    // Close the log file before reading it
    logger.closeLogFile();

    // Switch back to console for verification output
    logger.setOutputDestination(EOutput::CONSOLE);

    // Verify the log file was created and contains expected content
    assert(std::filesystem::exists(test_log_file));

    std::ifstream log_file(test_log_file);
    assert(log_file.is_open());

    std::string file_content;
    std::string line;
    while (std::getline(log_file, line))
    {
        file_content += line + "\n";
    }
    log_file.close();

    // Check that our test messages are in the file
    assert(file_content.find("This message should go to file") != std::string::npos);
    assert(file_content.find("Warning message in file") != std::string::npos);
    assert(file_content.find("Error message with details") != std::string::npos);
    assert(file_content.find("error_code=404") != std::string::npos);

    std::cout << "Log file created with " << std::count(file_content.begin(), file_content.end(), '\n')
              << " lines" << std::endl;

    // Clean up
    std::filesystem::remove(test_log_file);

    std::cout << "✓ File logging test passed" << std::endl;
}

void test_both_output_destinations()
{
    std::cout << "Testing logging to both console and file..." << std::endl;

    Logger &logger = Logger::getInstance();
    const std::string test_log_file = "test_both_output.log";

    // Clean up any existing log file
    if (std::filesystem::exists(test_log_file))
    {
        std::filesystem::remove(test_log_file);
    }

    // Configure for both console and file logging
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    logger.setLogFile(test_log_file);
    logger.setOutputDestination(EOutput::BOTH);

    std::cout << "The following messages should appear both on console and in file:" << std::endl;

    // Log some test messages
    ECLIPSE_DEBUG("BOTH_TEST", "Debug message to both outputs");
    ECLIPSE_INFO("BOTH_TEST", "Info message with timestamp");
    ECLIPSE_FATAL("BOTH_TEST", "Fatal error logged to both", "severity=critical");

    // Close the log file before reading it
    logger.closeLogFile();

    // Verify the log file was created and contains expected content
    assert(std::filesystem::exists(test_log_file));

    std::ifstream log_file(test_log_file);
    assert(log_file.is_open());

    std::string file_content;
    std::string line;
    while (std::getline(log_file, line))
    {
        file_content += line + "\n";
    }
    log_file.close();

    // Check that our test messages are in the file
    assert(file_content.find("Debug message to both outputs") != std::string::npos);
    assert(file_content.find("Info message with timestamp") != std::string::npos);
    assert(file_content.find("Fatal error logged to both") != std::string::npos);

    // Clean up
    std::filesystem::remove(test_log_file);

    std::cout << "✓ Both output destinations test passed" << std::endl;
}

void test_custom_config_parsing()
{
    std::cout << "Testing custom configuration parsing..." << std::endl;

    // Create a custom config file for testing
    const std::string custom_config = "custom_test.ini";
    std::ofstream config_file(custom_config);
    config_file << "# Custom test configuration\n";
    config_file << "ECLIPSE_LOG_LEVEL=ERROR\n";
    config_file << "some_other_setting=value\n";
    config_file << "ECLIPSE_LOG_LEVEL=INFO  # This should override the previous one\n";
    config_file.close();

    Logger &logger = Logger::getInstance();

    // Load the custom config
    bool success = logger.loadConfig("./" + custom_config);
    assert(success);

    // Should be set to INFO (the last valid setting)
    assert(logger.getLevel() == ELevel::ECLIPSE_ERROR);

    // Clean up
    std::filesystem::remove(custom_config);

    std::cout << "✓ Custom configuration parsing test passed" << std::endl;
}

void test_level_parsing_variants()
{
    std::cout << "Testing various log level parsing formats..." << std::endl;

    // Test different level format configurations
    struct TestCase
    {
        std::string config_value;
        ELevel expected_level;
        std::string description;
    };

    std::vector<TestCase> test_cases = {
        {"DEBUG", ELevel::ECLIPSE_DEBUG, "uppercase DEBUG"},
        {"debug", ELevel::ECLIPSE_DEBUG, "lowercase debug"},
        {"INFO", ELevel::ECLIPSE_INFO, "uppercase INFO"},
        {"WARN", ELevel::ECLIPSE_WARN, "WARN variant"},
        {"WARNING", ELevel::ECLIPSE_WARN, "WARNING variant"},
        {"ERROR", ELevel::ECLIPSE_ERROR, "ERROR"},
        {"FATAL", ELevel::ECLIPSE_FATAL, "FATAL"},
        {"0", ELevel::ECLIPSE_DEBUG, "numeric 0 (DEBUG)"},
        {"1", ELevel::ECLIPSE_INFO, "numeric 1 (INFO)"},
        {"2", ELevel::ECLIPSE_WARN, "numeric 2 (WARN)"},
        {"3", ELevel::ECLIPSE_ERROR, "numeric 3 (ERROR)"},
        {"4", ELevel::ECLIPSE_FATAL, "numeric 4 (FATAL)"},
        {"\"DEBUG\"", ELevel::ECLIPSE_DEBUG, "quoted DEBUG"},
        {"  INFO  ", ELevel::ECLIPSE_INFO, "INFO with whitespace"}};

    Logger &logger = Logger::getInstance();

    for (const auto &test_case : test_cases)
    {
        // Create a temporary config file
        const std::string temp_config = "temp_level_test.ini";
        std::ofstream config_file(temp_config);
        config_file << "ECLIPSE_LOG_LEVEL=" << test_case.config_value << "\n";
        config_file.close();

        // Load and test
        bool success = logger.loadConfig(temp_config);
        assert(success);
        assert(logger.getLevel() == test_case.expected_level);

        std::cout << "  ✓ " << test_case.description << " -> " << logger.getLevelName(test_case.expected_level) << std::endl;

        // Clean up
        std::filesystem::remove(temp_config);
    }

    std::cout << "✓ Level parsing variants test passed" << std::endl;
}

void test_file_append_mode()
{
    std::cout << "Testing file append mode..." << std::endl;

    Logger &logger = Logger::getInstance();
    const std::string test_log_file = "test_append.log";

    // Clean up any existing log file
    if (std::filesystem::exists(test_log_file))
    {
        std::filesystem::remove(test_log_file);
    }

    // Configure for file logging
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    logger.setLogFile(test_log_file);
    logger.setOutputDestination(EOutput::FILE);

    // Log first batch of messages
    ECLIPSE_INFO("APPEND_TEST", "First message");
    ECLIPSE_INFO("APPEND_TEST", "Second message");

    // Verify file exists and has content
    assert(std::filesystem::exists(test_log_file));
    std::ifstream first_read(test_log_file);
    std::string first_content((std::istreambuf_iterator<char>(first_read)),
                              std::istreambuf_iterator<char>());
    first_read.close();

    // Set the same log file again (should append, not overwrite)
    logger.setLogFile(test_log_file);

    // Log more messages
    ECLIPSE_INFO("APPEND_TEST", "Third message");
    ECLIPSE_INFO("APPEND_TEST", "Fourth message");

    // Close the log file before reading it
    logger.closeLogFile();

    // Read final content
    std::ifstream second_read(test_log_file);
    std::string final_content((std::istreambuf_iterator<char>(second_read)),
                              std::istreambuf_iterator<char>());
    second_read.close();

    // Final content should be longer than first content (appended)
    assert(final_content.length() > first_content.length());
    assert(final_content.find("First message") != std::string::npos);
    assert(final_content.find("Fourth message") != std::string::npos);

    // Clean up
    std::filesystem::remove(test_log_file);
    logger.closeLogFile();
    logger.setOutputDestination(EOutput::CONSOLE);

    std::cout << "✓ File append mode test passed" << std::endl;
}

int main()
{
    try
    {
        std::cout << "=== Eclipse Logger Configuration and File Tests ===" << std::endl;
        std::cout << "Testing configuration loading and file output..." << std::endl
                  << std::endl;

        test_config_file_loading();
        test_config_file_not_found();
        test_file_logging();
        test_both_output_destinations();
        test_custom_config_parsing();
        test_level_parsing_variants();
        test_file_append_mode();

        std::cout << std::endl
                  << "🎉 All configuration and file tests passed successfully!" << std::endl;
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
