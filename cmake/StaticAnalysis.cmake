function(enable_static_analysis target_name)
    if(ASYNC_STRINGS_ENABLE_CLANG_TIDY)
        find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)

        set(CLANG_TIDY_ARGS "${CLANG_TIDY_EXECUTABLE}")

        # I used MinGW for the build on Windows, 
        # so I need to configure the MinGW paths in order to use clang-tidy.
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
                list(APPEND CLANG_TIDY_ARGS
                    "--extra-arg-before=--target=x86_64-w64-windows-gnu"
                )
            endif()

            foreach(include_directory IN LISTS CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)
                list(APPEND CLANG_TIDY_ARGS
                    "--extra-arg-before=-isystem${include_directory}"
                )
            endforeach()
        endif()

        set_target_properties(${target_name} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_ARGS}"
        )
    endif()

    if(ASYNC_STRINGS_ENABLE_CPPCHECK)
        find_program(CPPCHECK_EXECUTABLE NAMES cppcheck REQUIRED)
        set_target_properties(${target_name} PROPERTIES
            CXX_CPPCHECK
                "${CPPCHECK_EXECUTABLE};--enable=warning,style,performance,portability;--suppress=*:*/include/utf8/*;--suppress=*:*/include/utf8.h"
        )
    endif()
endfunction()