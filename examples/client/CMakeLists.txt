add_executable(async_strings_handler_example main.cpp)
target_link_libraries(async_strings_handler_example
    PRIVATE AsyncStringsHandler::AsyncStringsHandler
)

set_project_warnings(async_strings_handler_example)

add_custom_command(TARGET async_strings_handler_example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:AsyncStringsHandler>
    $<TARGET_FILE_DIR:async_strings_handler_example>
)
