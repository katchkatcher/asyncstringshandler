#include <iostream>
#include "async_strings_handler/AsyncStringsHandler.hpp"

int main() {
    AsyncStringsHandler::StringsHandler hndlr(2);
    AsyncStringsHandler::TaskId id1 = hndlr.SubmitTask("Hello");
    AsyncStringsHandler::TaskId id2 = hndlr.SubmitTask("World");

    std::string result1 = hndlr.GetResult(id1);
    std::string result2 = hndlr.GetResult(id2);

    std::cout << "Result 1: " << result1 << std::endl;
    std::cout << "Result 2: " << result2 << std::endl;

    return 0;
}
