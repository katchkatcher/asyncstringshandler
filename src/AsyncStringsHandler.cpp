#include "async_strings_handler/AsyncStringsHandler.hpp"
#include "utf8.h"
#include <cctype>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <algorithm>
#include <thread>
#include <unordered_map>
#include <vector>

namespace AsyncStringsHandler {

class StringsHandler::Impl {
  public:
    struct Task {
        TaskId id{};
        std::string input;
    };

    std::mutex mtx;
    std::condition_variable tasksCv;
    std::condition_variable resultsCv;

    std::queue<Task> pendingTasks;
    std::unordered_map<TaskId, std::string> completedById;

    std::vector<std::thread> workers;
    TaskId nextTaskId = 1;
    bool stopping = false;
    const uint8_t operationDelayMs = 50;

    void WorkerLoop() {
        while (true) {
            Task task;
            {
                std::unique_lock<std::mutex> lock(mtx);

                tasksCv.wait(lock, [this] { return stopping || !pendingTasks.empty(); });

                /* If we are stopping and there are no more tasks, we can exit */
                if (stopping && pendingTasks.empty()) {
                    return;
                }

                task = std::move(pendingTasks.front());
                pendingTasks.pop();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(operationDelayMs));

            try {
                /* process the string */
                task.input = utf8ToLower(task.input);
            } catch (const std::exception& error) {
                task.input = std::string("UTF-8 processing error: ") + error.what();
            }

            {
                std::lock_guard<std::mutex> lock(mtx);
                completedById[task.id] = std::move(task.input);
            }
            resultsCv.notify_all();
        }
    }

    static uint32_t char32ToLower(uint32_t symbol) {
        /* ASCII processing */
        const auto difference = 0x20;
        const auto startLetterEn = 0x41;
        const auto endLetterEn = 0x5A;
        if (symbol >= startLetterEn && symbol <= endLetterEn) { // 'A' - 'Z'
            return symbol + difference;
        }

        /* russian processing */
        /* in unicode russian 'A' letter is 0x0410, 'a' is 0x0430. differense 0x20(32)*/
        const auto startLetterRus = 0x0410;
        const auto endLetterRus = 0x042F;

        if (symbol >= startLetterRus && symbol <= endLetterRus) {
            return symbol + difference;
        }

        /* 'Ё' is 0x0401 -> 'ё' is 0x0451 */
        const auto specialCapital = 0x0401;
        const auto specialLowercase = 0x0451;
        if (symbol == specialCapital) {
            return specialLowercase;
        }

        /* all other symbols just skip */
        return symbol;
    }

    std::string static utf8ToLower(const std::string& text) {
        if (!utf8::is_valid(text.begin(), text.end())) {
            throw std::invalid_argument("Input is not valid UTF-8");
        }

        std::u32string u32text;
        utf8::utf8to32(text.begin(), text.end(), std::back_inserter(u32text));

        std::transform(u32text.cbegin(), u32text.cend(), u32text.begin(), char32ToLower);

        /* back to UTF-8 */
        std::string result;
        utf8::utf32to8(u32text.begin(), u32text.end(), std::back_inserter(result));

        return result;
    }
};

StringsHandler::StringsHandler(std::size_t workerCount) : impl_(std::make_unique<Impl>()) {
    if (workerCount == 0) {
        workerCount = std::thread::hardware_concurrency();

        if (workerCount == 0) {
            workerCount = 1;
        }
    }
    impl_->workers.reserve(workerCount);

    for (std::size_t i = 0; i < workerCount; i++) {
        impl_->workers.emplace_back([this] { impl_->WorkerLoop(); });
    }
}

StringsHandler::~StringsHandler() { Shutdown(); }

void StringsHandler::Shutdown() {
    if (!impl_) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(impl_->mtx);
        impl_->stopping = true;
    }

    /* I only wake up the workers so that they can finish their
     *  tasks and the user receives the result, even after a shutdown.
     */
    impl_->tasksCv.notify_all();

    for (auto& worker : impl_->workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

TaskId StringsHandler::SubmitTask(std::string input) {
    TaskId taskId;

    {
        std::lock_guard<std::mutex> lock(impl_->mtx);
        if (impl_->stopping) {
            throw std::logic_error(
                "Error: Assigning a task after a shutdown has begun is prohibited.");
        }

        taskId = impl_->nextTaskId++;
        impl_->pendingTasks.push({taskId, std::move(input)});
    }

    impl_->tasksCv.notify_one();
    return taskId;
}

std::string StringsHandler::GetResult(TaskId taskId) {
    std::unique_lock<std::mutex> lock(impl_->mtx);

    if (taskId == 0 || taskId >= impl_->nextTaskId) {
        throw std::invalid_argument("Invalid Id Request");
    }

    impl_->resultsCv.wait(lock, [this, taskId] { return impl_->completedById.count(taskId); });

    auto node = impl_->completedById.extract(taskId);

    if (node.empty()) {
        return {};
    }

    /* node.mapped() returns T&, so RVO/NRVO doesn't apply. std::move prevents copying. */
    return std::move(node.mapped());
}

} // namespace AsyncStringsHandler
