add_executable(async_strings_handler_tests
    AsyncStringsHandlerTests.cpp
)

target_link_libraries(async_strings_handler_tests
    PRIVATE AsyncStringsHandler::AsyncStringsHandler
)

set_project_warnings(async_strings_handler_tests)

add_test(
    NAME AsyncStringsHandler.Tests
    COMMAND async_strings_handler_tests
)

add_custom_command(TARGET async_strings_handler_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:AsyncStringsHandler>
    $<TARGET_FILE_DIR:async_strings_handler_tests>
)
