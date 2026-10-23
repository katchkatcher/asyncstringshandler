add_executable(async_strings_handler_benchmark
    AsyncStringsHandlerBenchmark.cpp
)

target_link_libraries(async_strings_handler_benchmark
    PRIVATE AsyncStringsHandler::AsyncStringsHandler
)

set_project_warnings(async_strings_handler_benchmark)

add_custom_command(TARGET async_strings_handler_benchmark POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:AsyncStringsHandler>
    $<TARGET_FILE_DIR:async_strings_handler_benchmark>
)
