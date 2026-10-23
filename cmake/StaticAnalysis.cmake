function(enable_static_analysis target_name)
    if(ASYNC_STRINGS_ENABLE_CLANG_TIDY)
        find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
        set_target_properties(${target_name} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}"
        )
    endif()

    if(ASYNC_STRINGS_ENABLE_CPPCHECK)
        find_program(CPPCHECK_EXECUTABLE NAMES cppcheck REQUIRED)
        set_target_properties(${target_name} PROPERTIES
            CXX_CPPCHECK "${CPPCHECK_EXECUTABLE};--enable=warning,style,performance,portability"
        )
    endif()
endfunction()
