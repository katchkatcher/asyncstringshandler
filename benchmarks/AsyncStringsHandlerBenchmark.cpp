#include "async_strings_handler/AsyncStringsHandler.hpp"

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

std::size_t ReadTaskCount(int argc, char* argv[]) {
    if (argc < 2) {
        return 40;
    }

    const auto value = std::strtoull(argv[1], nullptr, 10);
    return value == 0 ? 40 : static_cast<std::size_t>(value);
}

void RunBenchmark(std::size_t workerCount, std::size_t taskCount) {
    AsyncStringsHandler::StringsHandler handler(workerCount);
    std::vector<AsyncStringsHandler::TaskId> taskIds;
    taskIds.reserve(taskCount);

    const auto started = Clock::now();

    for (std::size_t i = 0; i < taskCount; ++i) {
        taskIds.push_back(handler.SubmitTask("BENCHMARK_TASK_" +
                                             std::to_string(i)));
    }

    for (const auto taskId : taskIds) {
        const auto res = handler.GetResult(taskId);
        if(res != "benchmark_task_" + std::to_string(taskId - 1)) {
            throw std::runtime_error("Benchmark result is incorrect");
        }
    }

    const auto elapsed =
        std::chrono::duration<double, std::milli>(Clock::now() - started);
    const double tasksPerSecond =
        static_cast<double>(taskCount) / (elapsed.count() / 1000.0);

    std::cout << workerCount
              << " workers | "
              << std::setw(7) << taskCount
              << " tasks | "
              << std::fixed << std::setprecision(2)
              << std::setw(10) << elapsed.count()
              << " ms | "
              << std::setw(10) << tasksPerSecond
              << " tasks/s\n";
}

} // namespace

int main(int argc, char* argv[]) {
    const auto taskCount = ReadTaskCount(argc, argv);

    std::cout << "Tasks: " << taskCount << '\n';
    std::cout << "Workers   |   Tasks       |     Time      |   Throughput\n";

    for (const std::size_t workerCount : {1u, 2u, 4u, 8u, 16u, 32u, 64u}) {
        RunBenchmark(workerCount, taskCount);
    }

    return 0;
}
