#include "async_strings_handler/AsyncStringsHandler.hpp"
#include <cctype>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

namespace AsyncStringsHandler {

class StringsHandler::Impl {
public:
    struct Task {
        TaskId id;
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

    void WorkerLoop()
    {
        while (true)
        {
            Task task;
            {
                std::unique_lock<std::mutex> lock(mtx);

                tasksCv.wait(lock, [this]
                             { return stopping || !pendingTasks.empty(); });

                if (stopping && pendingTasks.empty())
                {
                    return;
                }

                task = std::move(pendingTasks.front());
                pendingTasks.pop();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            for (auto &ch : task.input)
            {
                ch = static_cast<char>(
                    std::tolower(static_cast<unsigned char>(ch))
                );
            }
            {
                std::lock_guard<std::mutex> lock(mtx);
                completedById[task.id] = std::move(task.input);
            }
            resultsCv.notify_all();
        }
    }
};

StringsHandler::StringsHandler(std::size_t workerCount) : impl_(std::make_unique<Impl>()) {
    if(workerCount == 0) {
        workerCount = std::thread::hardware_concurrency();

        if(workerCount == 0) {
            workerCount = 1;
        }
    }
    impl_->workers.reserve(workerCount);

    for(std::size_t i = 0; i < workerCount; i++) {
        impl_->workers.emplace_back([this] {
            impl_->WorkerLoop();
        });
    }
}

StringsHandler::~StringsHandler() {
    Shutdown();
}

void StringsHandler::Shutdown() {
    if(!impl_) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(impl_->mtx);
        impl_->stopping = true;
    }

    impl_->tasksCv.notify_all();
    impl_->resultsCv.notify_all();

    for(auto& worker : impl_->workers) {
        if(worker.joinable()) {
            worker.join();
        }
    }
}

TaskId StringsHandler::SubmitTask(std::string input) {
    TaskId taskId;

    {
        std::lock_guard<std::mutex> lock(impl_->mtx);   
        taskId = impl_->nextTaskId++;
        impl_->pendingTasks.push({taskId, std::move(input)});
    }

    impl_->tasksCv.notify_one();
    return taskId;
}

std::string StringsHandler::GetResult(TaskId taskId)
{
    std::unique_lock<std::mutex> lock(impl_->mtx);
    
    if(taskId == 0 || taskId >= impl_->nextTaskId) {
        return {};
    }

    impl_->resultsCv.wait(lock, [this, taskId]
                          { return impl_->completedById.count(taskId) || impl_->stopping; });

    auto node = impl_->completedById.extract(taskId);

    if (node.empty())
    {
        return {};
    }

    return std::move(node.mapped());
}

}
