#include "Eclipse/Logger.h"
#include "Eclipse/LogMacros.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <filesystem>

// Simple test framework
class TestRunner {
private:
    int totalTests = 0;
    int passedTests = 0;
    std::ostringstream output;

public:
    void test(const std::string& testName, bool condition) {
        totalTests++;
        output << "[TEST] " << testName << ": ";
        if (condition) {
            output << "PASSED\n";
            passedTests++;
        } else {
            output << "FAILED\n";
        }
    }

    bool runAll() {
        std::cout << output.str();
        std::cout << "\n=== Test Summary ===\n";
        std::cout << "Passed: " << passedTests << "/" << totalTests << std::endl;
        
        if (passedTests == totalTests) {
            std::cout << "All tests PASSED!\n" << std::endl;
            return true;
        } else {
            std::cout << "Some tests FAILED!\n" << std::endl;
            return false;
        }
    }
};

// Helper function to create test config files
void createTestConfigFiles() {
    // Create test .env file
    std::ofstream envFile("test_debug.env");
    envFile << "LOG_LEVEL=DEBUG\n";
    envFile << "APP_NAME=TestApp\n";
    envFile.close();

    std::ofstream envFile2("test_info.env");
    envFile2 << "LOG_LEVEL=INFO\n";
    envFile2.close();

    // Create test .ini files
    std::ofstream iniFile("test_warn.ini");
    iniFile << "[logging]\n";
    iniFile << "LOG_LEVEL=WARN\n";
    iniFile.close();

    std::ofstream iniFile2("test_error.ini");
    iniFile2 << "LOG_LEVEL=ERROR\n";
    iniFile2 << "timeout=30\n";
    iniFile2.close();

    // Create numeric test files
    std::ofstream numFile("test_numeric.env");
    numFile << "LOG_LEVEL=2\n";  // WARN level
    numFile.close();
}

// Helper function to clean up test files
void cleanupTestFiles() {
    std::filesystem::remove("test_debug.env");
    std::filesystem::remove("test_info.env");
    std::filesystem::remove("test_warn.ini");
    std::filesystem::remove("test_error.ini");
    std::filesystem::remove("test_numeric.env");
}

// Test the singleton pattern
void testSingletonPattern(TestRunner& runner) {
    Logger& logger1 = Logger::getInstance();
    Logger& logger2 = Logger::getInstance();
    
    runner.test("Singleton Pattern", &logger1 == &logger2);
}

// Test default log level
void testDefaultLogLevel(TestRunner& runner) {
    Logger& logger = Logger::getInstance();
    LogLevel defaultLevel = logger.getLogLevel();
    
    // Should default to DEBUG
    runner.test("Default Log Level is DEBUG", defaultLevel == LogLevel::DEBUG);
}

// Test log level setting and getting
void testLogLevelManagement(TestRunner& runner) {
    Logger& logger = Logger::getInstance();
    
    // Test setting different levels
    logger.setLogLevel(LogLevel::INFO);
    runner.test("Set/Get INFO Level", logger.getLogLevel() == LogLevel::INFO);
    
    logger.setLogLevel(LogLevel::WARN);
    runner.test("Set/Get WARN Level", logger.getLogLevel() == LogLevel::WARN);
    
    logger.setLogLevel(LogLevel::ERR);
    runner.test("Set/Get ERROR Level", logger.getLogLevel() == LogLevel::ERR);
    
    logger.setLogLevel(LogLevel::DEBUG);
    runner.test("Set/Get DEBUG Level", logger.getLogLevel() == LogLevel::DEBUG);
}

// Test .env file parsing
void testEnvFileParsing(TestRunner& runner) {
    Logger& logger = Logger::getInstance();
    
    // Test DEBUG level from .env
    bool loaded1 = logger.loadConfigFromFile("test_debug.env");
    runner.test("Load .env file (DEBUG)", loaded1 && logger.getLogLevel() == LogLevel::DEBUG);
    
    // Test INFO level from .env
    bool loaded2 = logger.loadConfigFromFile("test_info.env");
    runner.test("Load .env file (INFO)", loaded2 && logger.getLogLevel() == LogLevel::INFO);
    
    // Test numeric format
    bool loaded3 = logger.loadConfigFromFile("test_numeric.env");
    runner.test("Load .env file (Numeric WARN)", loaded3 && logger.getLogLevel() == LogLevel::WARN);
}

// Test .ini file parsing
void testIniFileParsing(TestRunner& runner) {
    Logger& logger = Logger::getInstance();
    
    // Test WARN level from .ini with [logging] section
    bool loaded1 = logger.loadConfigFromFile("test_warn.ini");
    runner.test("Load .ini file with [logging] section", loaded1 && logger.getLogLevel() == LogLevel::WARN);
    
    // Test ERROR level from standalone .ini
    bool loaded2 = logger.loadConfigFromFile("test_error.ini");
    runner.test("Load standalone .ini file", loaded2 && logger.getLogLevel() == LogLevel::ERR);
}

// Test error handling
void testErrorHandling(TestRunner& runner) {
    Logger& logger = Logger::getInstance();
    
    // Test non-existent file
    bool loaded1 = logger.loadConfigFromFile("nonexistent.env");
    runner.test("Handle non-existent file", !loaded1);
    
    // Test unsupported extension
    bool loaded2 = logger.loadConfigFromFile("test.txt");
    runner.test("Handle unsupported extension", !loaded2);
    
    // Test empty path
    bool loaded3 = logger.loadConfigFromFile("");
    runner.test("Handle empty path", !loaded3);
}

// Test macro compilation and basic functionality
void testMacroCompilation(TestRunner& runner) {
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(LogLevel::DEBUG);
    
    // Capture stdout to verify macros compile and run
    std::ostringstream capturedOutput;
    std::streambuf* originalCoutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(capturedOutput.rdbuf());
    
    try {
        // Test basic macros
        LOG_DEBUG("TEST", "Debug test message");
        LOG_INFO("TEST", "Info test message");
        LOG_WARNING("TEST", "Warning test message");
        LOG_ERROR("TEST", "Error test message");
        
        // Test detailed macros
        LOG_DEBUG_DETAILS("TEST", "Debug with details", "Additional info");
        LOG_INFO_DETAILS("TEST", "Info with details", "Additional info");
        LOG_WARNING_DETAILS("TEST", "Warning with details", "Additional info");
        LOG_ERROR_DETAILS("TEST", "Error with details", "Additional info");
        
        std::cout.rdbuf(originalCoutBuffer);
        
        std::string output = capturedOutput.str();
        bool hasContent = !output.empty();
        bool hasDebug = output.find("debug") != std::string::npos;
        bool hasInfo = output.find("info") != std::string::npos;
        bool hasWarning = output.find("warn") != std::string::npos;
        bool hasError = output.find("error") != std::string::npos;
        
        runner.test("Macros compile and execute", hasContent);
        runner.test("Debug macro works", hasDebug);
        runner.test("Info macro works", hasInfo);
        runner.test("Warning macro works", hasWarning);
        runner.test("Error macro works", hasError);
        
    } catch (...) {
        std::cout.rdbuf(originalCoutBuffer);
        runner.test("Macros compile and execute", false);
        runner.test("Debug macro works", false);
        runner.test("Info macro works", false);
        runner.test("Warning macro works", false);
        runner.test("Error macro works", false);
    }
}

// Test log level filtering
void testLogLevelFiltering(TestRunner& runner) {
    Logger& logger = Logger::getInstance();
    
    // Capture stdout
    std::ostringstream capturedOutput;
    std::streambuf* originalCoutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(capturedOutput.rdbuf());
    
    // Set to WARN level - should filter out DEBUG and INFO
    logger.setLogLevel(LogLevel::WARN);
    
    LOG_DEBUG("FILTER", "This should be filtered");
    LOG_INFO("FILTER", "This should be filtered");
    LOG_WARNING("FILTER", "This should appear");
    LOG_ERROR("FILTER", "This should appear");
    
    std::cout.rdbuf(originalCoutBuffer);
    std::string output = capturedOutput.str();
    
    bool noDebug = output.find("This should be filtered") == std::string::npos;
    bool hasWarning = output.find("This should appear") != std::string::npos;
    
    runner.test("Log level filtering works", noDebug && hasWarning);
}

int main() {
    std::cout << "=== Eclipse Logger Test Suite ===\n" << std::endl;
    
    TestRunner runner;
    
    // Create test files
    createTestConfigFiles();
    
    try {
        // Run all tests
        testSingletonPattern(runner);
        testDefaultLogLevel(runner);
        testLogLevelManagement(runner);
        testEnvFileParsing(runner);
        testIniFileParsing(runner);
        testErrorHandling(runner);
        testMacroCompilation(runner);
        testLogLevelFiltering(runner);
        
        // Clean up
        cleanupTestFiles();
        
        // Show results
        bool allPassed = runner.runAll();
        
        return allPassed ? 0 : 1;
        
    } catch (const std::exception& e) {
        cleanupTestFiles();
        std::cerr << "Test execution failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        cleanupTestFiles();
        std::cerr << "Test execution failed with unknown exception" << std::endl;
        return 1;
    }
}
