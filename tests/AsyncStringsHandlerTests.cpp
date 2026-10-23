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

void TestRepeatedShutdown() {
    AsyncStringsHandler::StringsHandler handler(2);
    handler.Shutdown();
    handler.Shutdown();
}

} // namespace

int main() {
    try {
        TestSingleTask();
        TestMultipleTasksById();
        TestEmptyString();
        TestConcurrentClients();
        TestRepeatedShutdown();
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All tests passed\n";
    return 0;
}
