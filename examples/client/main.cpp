#include "async_strings_handler/AsyncStringsHandler.hpp"

#include <charconv>
#include <cstddef>
#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

constexpr std::size_t kTasksPerClient = 50;

std::size_t ReadClientCount(int argc, char* argv[]) {
    if (argc != 2) {
        throw std::invalid_argument("Usage: async_strings_handler_example <client-count>");
    }

    const std::string argument = argv[1];
    std::size_t clientCount{};
    const auto [end, error] = std::from_chars(
        argument.data(), argument.data() + argument.size(), clientCount);
    if (error != std::errc{} || end != argument.data() + argument.size() ||
        clientCount == 0) {
        throw std::invalid_argument("client-count must be a positive integer");
    }

    return clientCount;
}

} // namespace

int main(int argc, char* argv[]) {
    try {
#ifdef _WIN32
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
#endif

        const std::size_t clientCount = ReadClientCount(argc, argv);
        AsyncStringsHandler::StringsHandler handler;
        std::vector<std::string> results;
        std::mutex resultsMutex;
        std::exception_ptr clientError;
        std::vector<std::thread> clients;
        clients.reserve(clientCount);

        for (std::size_t client = 0; client < clientCount; ++client) {
            clients.emplace_back([client, &handler, &results, &resultsMutex,
                                  &clientError] {
                try {
                    for (std::size_t task = 0; task < kTasksPerClient; ++task) {
                        const std::string input =
                            "CLIENT_" + std::to_string(client) +
                            "_TASK_" + std::to_string(task);
                        const auto taskId = handler.SubmitTask(input);
                        const auto result = handler.GetResult(taskId);

                        std::lock_guard<std::mutex> lock(resultsMutex);
                        results.push_back(result);
                    }
                } catch (...) {
                    std::lock_guard<std::mutex> lock(resultsMutex);
                    if (!clientError) {
                        clientError = std::current_exception();
                    }
                }
            });
        }

        for (auto& client : clients) {
            client.join();
        }

        if (clientError) {
            std::rethrow_exception(clientError);
        }

        std::cout << "Clients: " << clientCount
                  << ", tasks per client: " << kTasksPerClient
                  << ", results: " << results.size() << '\n';
        return results.size() == clientCount * kTasksPerClient ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
