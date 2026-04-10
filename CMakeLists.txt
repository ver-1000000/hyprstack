cmake_minimum_required(VERSION 3.24)

project(hyprstack LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(HYPRLAND REQUIRED IMPORTED_TARGET hyprland)

set(HYPRSTACK_WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
)

set(HYPRSTACK_CORE_SOURCES
    src/domain/workspace_stack.cpp
    src/application/stack_store.cpp
    src/application/state_sync.cpp
    src/application/query_service.cpp
    src/application/dispatch_service.cpp
    src/application/plugin_state.cpp
    src/interface/query_parser.cpp
    src/interface/json_renderer.cpp
)

add_library(hyprstack_plugin MODULE
    ${HYPRSTACK_CORE_SOURCES}
    src/infra/hyprland/observer.cpp
    src/infra/hyprland/executor.cpp
    src/infra/hyprland/event_bridge.cpp
    src/plugin/plugin_app.cpp
    src/plugin/entrypoint.cpp
)

target_include_directories(hyprstack_plugin PRIVATE include)
target_compile_options(hyprstack_plugin PRIVATE ${HYPRSTACK_WARNING_FLAGS})
target_link_libraries(hyprstack_plugin PRIVATE PkgConfig::HYPRLAND)
set_target_properties(hyprstack_plugin PROPERTIES
    OUTPUT_NAME hyprstack
    PREFIX ""
)

add_executable(tests_runner
    ${HYPRSTACK_CORE_SOURCES}
    tests/workspace_stack_test.cpp
    tests/plugin_state_test.cpp
    tests/stack_dispatcher_test.cpp
    tests/query_command_test.cpp
    tests/application/state_sync_test.cpp
    vendor/catch2/catch_amalgamated.cpp
)

target_include_directories(tests_runner PRIVATE include vendor/catch2)
target_compile_options(tests_runner PRIVATE ${HYPRSTACK_WARNING_FLAGS})
set_target_properties(tests_runner PROPERTIES OUTPUT_NAME tests-runner)
