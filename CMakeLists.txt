cmake_minimum_required(VERSION 3.20)

project(AsyncStringsHandler
    VERSION 1.0.0
    DESCRIPTION "Asynchronous string processing library"
    LANGUAGES CXX
)

option(ASYNC_STRINGS_BUILD_EXAMPLES "Build example applications" ON)
option(ASYNC_STRINGS_BUILD_TESTS "Build tests" ON)
option(ASYNC_STRINGS_BUILD_BENCHMARKS "Build benchmarks" ON)
option(ASYNC_STRINGS_ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)
option(ASYNC_STRINGS_ENABLE_CPPCHECK "Run Cppcheck during compilation" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(cmake/StaticAnalysis.cmake)

add_library(AsyncStringsHandler SHARED
    src/AsyncStringsHandler.cpp
)

add_library(AsyncStringsHandler::AsyncStringsHandler ALIAS AsyncStringsHandler)

target_compile_features(AsyncStringsHandler PUBLIC cxx_std_17)
target_include_directories(AsyncStringsHandler
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_definitions(AsyncStringsHandler PRIVATE ASYNC_STRINGS_HANDLER_EXPORTS)

if(NOT WIN32)
    # Im keeping method hiding for Linux/macOS.
    set_target_properties(AsyncStringsHandler PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

include(cmake/CompilerWarnings.cmake)
set_project_warnings(AsyncStringsHandler)
enable_static_analysis(AsyncStringsHandler)

if(ASYNC_STRINGS_BUILD_EXAMPLES)
    add_subdirectory(examples/client)
endif()

if(ASYNC_STRINGS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(ASYNC_STRINGS_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

install(TARGETS AsyncStringsHandler
    EXPORT AsyncStringsHandlerTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT AsyncStringsHandlerTargets
    FILE AsyncStringsHandlerTargets.cmake
    NAMESPACE AsyncStringsHandler::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AsyncStringsHandler
)

configure_package_config_file(
    cmake/AsyncStringsHandlerConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AsyncStringsHandlerConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AsyncStringsHandler
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AsyncStringsHandlerConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AsyncStringsHandlerConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AsyncStringsHandlerConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AsyncStringsHandler
)
