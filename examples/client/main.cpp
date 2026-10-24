#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif
#include "async_strings_handler/AsyncStringsHandler.hpp"

int main() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "Number of worker cores: ";
    size_t workingCores{};
    std::cin >> workingCores;

    AsyncStringsHandler::StringsHandler handler(workingCores);

    std::string input;
    while (std::cin >> input) {
        const auto id = handler.SubmitTask(input);
        std::cout << "Result: " << handler.GetResult(id) << std::endl;
    }
    return 0;
}
