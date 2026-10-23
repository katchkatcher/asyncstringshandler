#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#if defined(_WIN32)
    #if defined(ASYNC_STRINGS_HANDLER_EXPORTS)
        #define STRINGS_HANDLER_LIB_API __declspec(dllexport)
    #else
        #define STRINGS_HANDLER_LIB_API __declspec(dllimport)
    #endif
#else
    #define STRINGS_HANDLER_LIB_API __attribute__((visibility("default")))
#endif

namespace AsyncStringsHandler {

using TaskId = uint64_t;

class STRINGS_HANDLER_LIB_API StringsHandler {
public:
    explicit StringsHandler(std::size_t workerCount = 0);
    ~StringsHandler();

    StringsHandler(const StringsHandler&) = delete;
    StringsHandler& operator=(const StringsHandler&) = delete;

    TaskId SubmitTask(std::string input);
    std::string GetResult(TaskId taskId);
    void Shutdown();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}
