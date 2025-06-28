#include "Eclipse/Logger.h"
#include "Eclipse/LogMacros.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <sstream>

// Test thread safety of the logger
class ThreadSafetyTester
{
private:
    int totalTests = 0;
    int passedTests = 0;
    std::ostringstream output;

public:
    void test(const std::string &testName, bool condition)
    {
        totalTests++;
        output << "[THREAD_TEST] " << testName << ": ";
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
        std::cout << "\n=== Thread Safety Test Summary ===\n";
        std::cout << "Passed: " << passedTests << "/" << totalTests << std::endl;

        if (passedTests == totalTests)
        {
            std::cout << "All thread safety tests PASSED!\n"
                      << std::endl;
            return true;
        }
        else
        {
            std::cout << "Some thread safety tests FAILED!\n"
                      << std::endl;
            return false;
        }
    }
};

void testConcurrentLogging(ThreadSafetyTester &runner)
{
    Logger &logger = Logger::getInstance();
    logger.setLogLevel(LogLevel::ECLIPSE_DEBUG);

    const int numThreads = 10;
    const int messagesPerThread = 100;
    std::atomic<int> completedThreads{0};
    std::atomic<bool> testFailed{false};

    // Capture output
    std::ostringstream capturedOutput;
    std::streambuf *originalCoutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(capturedOutput.rdbuf());

    std::vector<std::thread> threads;

    // Create multiple threads that log concurrently
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([i, messagesPerThread, &completedThreads, &testFailed]()
                             {
            try {
                for (int j = 0; j < messagesPerThread; ++j) {
                    LOG_INFO("THREAD" + std::to_string(i), "Message " + std::to_string(j));
                    
                    // Small delay to increase chance of race conditions
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
                completedThreads++;
            } catch (...) {
                testFailed = true;
            } });
    }

    // Wait for all threads to complete
    for (auto &thread : threads)
    {
        thread.join();
    }

    std::cout.rdbuf(originalCoutBuffer);
    std::string output = capturedOutput.str();

    bool allCompleted = (completedThreads == numThreads);
    bool noFailures = !testFailed.load();
    bool hasOutput = !output.empty();

    runner.test("Concurrent logging completed", allCompleted && noFailures);
    runner.test("Concurrent logging produced output", hasOutput);
}

void testConcurrentLevelChanges(ThreadSafetyTester &runner)
{
    Logger &logger = Logger::getInstance();

    std::atomic<bool> testFailed{false};
    std::atomic<int> completedOperations{0};
    const int numOperations = 1000;

    // Thread that changes log levels
    std::thread levelChanger([&logger, &testFailed, &completedOperations, numOperations]()
                             {
        try {
            for (int i = 0; i < numOperations; ++i) {            LogLevel levels[] = {LogLevel::ECLIPSE_DEBUG, LogLevel::ECLIPSE_INFO, LogLevel::ECLIPSE_WARN, LogLevel::ECLIPSE_ERROR};
            logger.setLogLevel(levels[i % 4]);
                completedOperations++;
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        } catch (...) {
            testFailed = true;
        } });

    // Thread that reads log levels
    std::thread levelReader([&logger, &testFailed, &completedOperations, numOperations]()
                            {
        try {
            for (int i = 0; i < numOperations; ++i) {
                volatile LogLevel level = logger.getLogLevel();
                (void)level; // Suppress unused variable warning
                completedOperations++;
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        } catch (...) {
            testFailed = true;
        } });

    levelChanger.join();
    levelReader.join();

    bool noFailures = !testFailed.load();
    bool allCompleted = (completedOperations >= numOperations); // At least one set completed

    runner.test("Concurrent level changes safe", noFailures && allCompleted);
}

void testSingletonThreadSafety(ThreadSafetyTester &runner)
{
    const int numThreads = 20;
    std::vector<Logger *> instances(numThreads);
    std::atomic<bool> testFailed{false};
    std::vector<std::thread> threads;

    // Multiple threads trying to get the singleton instance
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([i, &instances, &testFailed]()
                             {
            try {
                instances[i] = &Logger::getInstance();
            } catch (...) {
                testFailed = true;
            } });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    // Check that all instances are the same
    bool allSame = true;
    Logger *first = instances[0];
    for (int i = 1; i < numThreads; ++i)
    {
        if (instances[i] != first)
        {
            allSame = false;
            break;
        }
    }

    bool noFailures = !testFailed.load();
    runner.test("Singleton thread safety", noFailures && allSame);
}

int main()
{
    std::cout << "=== Eclipse Logger Thread Safety Tests ===\n"
              << std::endl;

    ThreadSafetyTester runner;

    try
    {
        testConcurrentLogging(runner);
        testConcurrentLevelChanges(runner);
        testSingletonThreadSafety(runner);

        bool allPassed = runner.runAll();
        return allPassed ? 0 : 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Thread safety test execution failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Thread safety test execution failed with unknown exception" << std::endl;
        return 1;
    }
}
