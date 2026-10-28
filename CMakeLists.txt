cmake_minimum_required(VERSION 3.24)

project(hyprstack LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
# hyprpm reuses the build directory across compositor upgrades.
unset(__pkg_config_checked_HYPRLAND CACHE)
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
if(HYPRLAND_VERSION VERSION_GREATER_EQUAL "0.57.0")
    target_compile_definitions(hyprstack_plugin PRIVATE HYPRSTACK_HYPRLAND_057)
    target_compile_features(hyprstack_plugin PRIVATE cxx_std_26)
endif()
target_compile_options(hyprstack_plugin PRIVATE ${HYPRSTACK_WARNING_FLAGS})
target_link_libraries(hyprstack_plugin PRIVATE PkgConfig::HYPRLAND)
# version.h describes Hyprland's original build, not necessarily the dependencies
# used to rebuild this plugin. hyprpm can reuse it after a dependency-only update.
foreach(dependency IN ITEMS aquamarine hyprutils hyprgraphics hyprcursor hyprlang)
    unset(__pkg_config_checked_ABI_${dependency} CACHE)
    pkg_check_modules(ABI_${dependency} REQUIRED IMPORTED_TARGET ${dependency})
    target_link_libraries(hyprstack_plugin PRIVATE PkgConfig::ABI_${dependency})
    string(TOUPPER "${dependency}" dependency_upper)
    target_compile_definitions(hyprstack_plugin PRIVATE
        "HYPRSTACK_${dependency_upper}_VERSION=\"${ABI_${dependency}_VERSION}\"")
endforeach()
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

enable_testing()
add_test(NAME unit COMMAND tests_runner)
add_test(NAME dependency_abi_refresh COMMAND "${CMAKE_COMMAND}"
    "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
    "-DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/abi-refresh-test"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/dependency_abi_refresh.cmake")
