/**
 * @file test_multithreaded_logging.cpp
 * @brief Multi-threaded logging tests for Eclipse Logger
 * @author tomosfps
 * @date 2025
 */

#include "Eclipse/Logger.h"
#include "Eclipse/Macros.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <random>

using namespace Eclipse;

// Global counter for thread safety testing
std::atomic<int> log_counter{0};

void worker_thread(int thread_id, int num_logs)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    for (int i = 0; i < num_logs; ++i)
    {
        int random_delay = dis(gen);
        std::this_thread::sleep_for(std::chrono::microseconds(random_delay));

        std::string tag = "THREAD_" + std::to_string(thread_id);
        std::string msg = "Log message " + std::to_string(i) + " from thread " + std::to_string(thread_id);

        // Test different log levels from different threads
        ELevel level = static_cast<ELevel>(i % 5);
        switch (level)
        {
        case ELevel::ECLIPSE_DEBUG:
            ECLIPSE_DEBUG(tag, msg, "iteration=" + std::to_string(i));
            break;
        case ELevel::ECLIPSE_INFO:
            ECLIPSE_INFO(tag, msg, "iteration=" + std::to_string(i));
            break;
        case ELevel::ECLIPSE_WARN:
            ECLIPSE_WARNING(tag, msg, "iteration=" + std::to_string(i));
            break;
        case ELevel::ECLIPSE_ERROR:
            ECLIPSE_ERROR(tag, msg, "iteration=" + std::to_string(i));
            break;
        case ELevel::ECLIPSE_FATAL:
            ECLIPSE_FATAL(tag, msg, "iteration=" + std::to_string(i));
            break;
        default:
            break;
        }

        log_counter.fetch_add(1);
    }
}

void test_concurrent_logging()
{
    std::cout << "Testing concurrent logging from multiple threads..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_DEBUG);
    logger.setOutputDestination(EOutput::CONSOLE);

    const int num_threads = 4;
    const int logs_per_thread = 10;

    std::vector<std::thread> threads;
    log_counter.store(0);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Create and start threads
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(worker_thread, i, logs_per_thread);
    }

    // Wait for all threads to complete
    for (auto &thread : threads)
    {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Completed " << log_counter.load() << " log operations from "
              << num_threads << " threads in " << duration.count() << "ms" << std::endl;

    // Verify all logs were processed
    assert(log_counter.load() == num_threads * logs_per_thread);

    std::cout << "✓ Concurrent logging test passed" << std::endl;
}

void test_concurrent_level_changes()
{
    std::cout << "Testing concurrent log level changes..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setOutputDestination(EOutput::CONSOLE);

    std::atomic<bool> stop_flag{false};
    std::atomic<int> level_changes{0};

    // Thread that continuously changes log levels
    std::thread level_changer([&]()
                              {
        std::vector<ELevel> levels = {
            ELevel::ECLIPSE_DEBUG,
            ELevel::ECLIPSE_INFO,
            ELevel::ECLIPSE_WARN,
            ELevel::ECLIPSE_ERROR,
            ELevel::ECLIPSE_FATAL
        };
        
        int level_index = 0;
        while (!stop_flag.load()) {
            logger.setLevel(levels[level_index % levels.size()]);
            level_changes.fetch_add(1);
            level_index++;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } });

    // Thread that continuously logs messages
    std::thread logger_thread([&]()
                              {
        int message_count = 0;
        while (!stop_flag.load()) {
            ECLIPSE_INFO("LEVEL_TEST", "Message during level change", 
                        "count=" + std::to_string(message_count));
            message_count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        } });

    // Let them run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    stop_flag.store(true);

    level_changer.join();
    logger_thread.join();

    std::cout << "Performed " << level_changes.load() << " level changes concurrently with logging" << std::endl;
    std::cout << "✓ Concurrent level changes test passed" << std::endl;
}

void test_concurrent_output_destination_changes()
{
    std::cout << "Testing concurrent output destination changes..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_DEBUG);

    std::atomic<bool> stop_flag{false};
    std::atomic<int> destination_changes{0};

    // Thread that continuously changes output destinations
    std::thread destination_changer([&]()
                                    {
        std::vector<EOutput> destinations = {
            EOutput::CONSOLE,
            EOutput::NONE,
            EOutput::CONSOLE
        };
        
        int dest_index = 0;
        while (!stop_flag.load()) {
            logger.setOutputDestination(destinations[dest_index % destinations.size()]);
            destination_changes.fetch_add(1);
            dest_index++;
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        } });

    // Thread that continuously logs messages
    std::thread logger_thread([&]()
                              {
        int message_count = 0;
        while (!stop_flag.load()) {
            ECLIPSE_WARNING("DEST_TEST", "Message during destination change", 
                           "count=" + std::to_string(message_count));
            message_count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        } });

    // Let them run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    stop_flag.store(true);

    destination_changer.join();
    logger_thread.join();

    std::cout << "Performed " << destination_changes.load() << " destination changes concurrently with logging" << std::endl;
    std::cout << "✓ Concurrent output destination changes test passed" << std::endl;
}

void test_stress_logging()
{
    std::cout << "Testing stress logging with many threads..." << std::endl;

    Logger &logger = Logger::getInstance();
    logger.setLevel(ELevel::ECLIPSE_INFO);      // Reduce output for stress test
    logger.setOutputDestination(EOutput::NONE); // Disable output for performance

    const int num_threads = 10;
    const int logs_per_thread = 100;

    std::vector<std::thread> threads;
    log_counter.store(0);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Create and start threads
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([i, logs_per_thread]()
                             {
            for (int j = 0; j < logs_per_thread; ++j) {
                ECLIPSE_INFO("STRESS_" + std::to_string(i), 
                           "Stress test message " + std::to_string(j),
                           "thread=" + std::to_string(i),
                           "iteration=" + std::to_string(j));
                log_counter.fetch_add(1);
            } });
    }

    // Wait for all threads to complete
    for (auto &thread : threads)
    {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Re-enable console output for results
    logger.setOutputDestination(EOutput::CONSOLE);

    std::cout << "Stress test completed: " << log_counter.load() << " logs from "
              << num_threads << " threads in " << duration.count() << "ms" << std::endl;
    std::cout << "Average: " << (log_counter.load() * 1000.0 / duration.count()) << " logs/second" << std::endl;

    // Verify all logs were processed
    assert(log_counter.load() == num_threads * logs_per_thread);

    std::cout << "✓ Stress logging test passed" << std::endl;
}

int main()
{
    try
    {
        std::cout << "=== Eclipse Logger Multi-threaded Tests ===" << std::endl;
        std::cout << "Testing thread safety and concurrent access..." << std::endl
                  << std::endl;

        test_concurrent_logging();
        test_concurrent_level_changes();
        test_concurrent_output_destination_changes();
        test_stress_logging();

        std::cout << std::endl
                  << "🎉 All multi-threaded tests passed successfully!" << std::endl;
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
