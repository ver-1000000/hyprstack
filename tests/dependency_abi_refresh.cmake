# Configure only: synthetic dependency versions must never produce a loaded plugin.
find_program(PKG_CONFIG_EXECUTABLE NAMES pkg-config REQUIRED)
execute_process(COMMAND "${PKG_CONFIG_EXECUTABLE}" --variable=pcfiledir aquamarine
    OUTPUT_VARIABLE pc_dir OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
file(READ "${pc_dir}/aquamarine.pc" original_pc)
file(MAKE_DIRECTORY "${TEST_DIR}/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "${TEST_DIR}/pkgconfig:$ENV{PKG_CONFIG_PATH}")

# Deliberately differ from version.h and reuse the same CMake cache.
foreach(version IN ITEMS 0.98.0 0.99.0)
    string(REGEX REPLACE "Version:[^\n]*" "Version: ${version}" fixture "${original_pc}")
    file(WRITE "${TEST_DIR}/pkgconfig/aquamarine.pc" "${fixture}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -S "${SOURCE_DIR}" -B "${TEST_DIR}/build"
        COMMAND_ERROR_IS_FATAL ANY OUTPUT_QUIET)
    file(READ "${TEST_DIR}/build/compile_commands.json" commands)
    string(JSON command GET "${commands}" 0 command)
    string(FIND "${command}" "HYPRSTACK_AQUAMARINE_VERSION=\\\"${version}\\\"" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Plugin did not pick up dependency version ${version}")
    endif()
endforeach()

# The same cache must also switch the integration API when Hyprland is upgraded
# or downgraded. Only configure; these synthetic headers are never compiled.
execute_process(COMMAND "${PKG_CONFIG_EXECUTABLE}" --variable=pcfiledir hyprland
    OUTPUT_VARIABLE hyprland_pc_dir OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
file(READ "${hyprland_pc_dir}/hyprland.pc" original_hyprland_pc)
foreach(version IN ITEMS 0.56.2 0.57.0 0.56.2)
    string(REGEX REPLACE "Version:[^\n]*" "Version: ${version}" fixture "${original_hyprland_pc}")
    file(WRITE "${TEST_DIR}/pkgconfig/hyprland.pc" "${fixture}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -S "${SOURCE_DIR}" -B "${TEST_DIR}/build"
        COMMAND_ERROR_IS_FATAL ANY OUTPUT_QUIET)
    file(READ "${TEST_DIR}/build/compile_commands.json" commands)
    string(JSON command GET "${commands}" 0 command)
    string(FIND "${command}" "HYPRSTACK_HYPRLAND_057" position)
    if(version VERSION_GREATER_EQUAL "0.57.0")
        if(position EQUAL -1)
            message(FATAL_ERROR "Plugin did not switch to the Hyprland 0.57 API")
        endif()
    elseif(NOT position EQUAL -1)
        message(FATAL_ERROR "Plugin did not switch back to the Hyprland 0.56 API")
    endif()
endforeach()
