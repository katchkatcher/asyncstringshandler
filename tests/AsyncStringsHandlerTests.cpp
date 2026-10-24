#include "async_strings_handler/AsyncStringsHandler.hpp"

#include <exception>
#include <iostream>
#include <atomic>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

void Require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void TestSingleTask() {
    AsyncStringsHandler::StringsHandler handler(1);

    const auto taskId = handler.SubmitTask("HeLLo");
    const auto result = handler.GetResult(taskId);

    Require(result == "hello", "single task result is incorrect");
}

void TestMultipleTasksById() {
    AsyncStringsHandler::StringsHandler handler(2);

    const auto firstId = handler.SubmitTask("FIRST");
    const auto secondId = handler.SubmitTask("SECOND");

    Require(handler.GetResult(secondId) == "second",
            "second task result is incorrect");
    Require(handler.GetResult(firstId) == "first",
            "first task result is incorrect");
}

void TestEmptyString() {
    AsyncStringsHandler::StringsHandler handler(1);

    const auto taskId = handler.SubmitTask("");

    Require(handler.GetResult(taskId).empty(),
            "empty input should produce an empty result");
}

void TestConcurrentClients() {
    constexpr std::size_t clientCount = 4;
    constexpr std::size_t tasksPerClient = 4;

    AsyncStringsHandler::StringsHandler handler(4);
    std::vector<std::thread> clients;
    clients.reserve(clientCount);
    std::atomic<std::size_t> failures{0};

    for (std::size_t client = 0; client < clientCount; ++client) {
        clients.emplace_back([client, &handler, &failures] {
            for (std::size_t task = 0; task < tasksPerClient; ++task) {
                const std::string input =
                    "CLIENT_" + std::to_string(client) +
                    "_TASK_" + std::to_string(task);
                const std::string expected =
                    "client_" + std::to_string(client) +
                    "_task_" + std::to_string(task);
                const auto taskId = handler.SubmitTask(input);
                const auto result = handler.GetResult(taskId);

                if (result != expected) {
                    ++failures;
                }
            }
        });
    }

    for (auto& client : clients) {
        client.join();
    }

    Require(failures == 0, "concurrent client result is incorrect");
}

void TestUtf8LowerCase() {
    AsyncStringsHandler::StringsHandler handler(1);
    const auto taskId = handler.SubmitTask("ПРИВЕТ");
    const auto result = handler.GetResult(taskId);
    Require(result == "привет", "utf-8 lowercase conversion is incorrect");
}

void TestInvalidUtf8DoesNotTerminate() {
    AsyncStringsHandler::StringsHandler handler(1);
    const std::string invalidUtf8 = std::string("\xFF\xFE", 2);
    const auto taskId = handler.SubmitTask(invalidUtf8);
    try {
        static_cast<void>(handler.GetResult(taskId));
        throw std::runtime_error("invalid utf-8 did not produce an error");
    } catch (const std::invalid_argument&) {
        // Expected error from the worker.
    }
}

void TestRepeatedGetResult() {
    AsyncStringsHandler::StringsHandler handler(1);
    const auto taskId = handler.SubmitTask("HELLO");
    Require(handler.GetResult(taskId) == "hello",
            "first result retrieval is incorrect");

    try {
        static_cast<void>(handler.GetResult(taskId));
        throw std::runtime_error("repeated GetResult did not throw");
    } catch (const std::logic_error&) {
        // Expected error for a consumed result.
    }
}

void TestRepeatedShutdown() {
    AsyncStringsHandler::StringsHandler handler(2);
    handler.Shutdown();
    handler.Shutdown();
}

void TestGetResultAfterShutdown() {
    AsyncStringsHandler::StringsHandler handler(2);
    const auto id = handler.SubmitTask("HELLO");
    handler.Shutdown();
    const auto result = handler.GetResult(id);
    Require(result == "hello", "invalid result after shutdown");
}

void TestSubmitAfterShutdown() {
    AsyncStringsHandler::StringsHandler handler(2);
    handler.Shutdown();
    try {
        handler.SubmitTask("HELLO");
        throw std::runtime_error("SubmitTask after shutdown did not throw");
    } catch (const std::logic_error&) {
        // Expected exception
    } catch (...) {
        throw std::runtime_error("SubmitTask after shutdown threw unexpected exception");
    }
}

} // namespace

int main() {
    try {
        TestSingleTask();
        TestMultipleTasksById();
        TestEmptyString();
        TestUtf8LowerCase();
        TestInvalidUtf8DoesNotTerminate();
        TestRepeatedGetResult();
        TestConcurrentClients();
        TestRepeatedShutdown();
        TestGetResultAfterShutdown();
        TestSubmitAfterShutdown();
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All tests passed\n";
    return 0;
}
