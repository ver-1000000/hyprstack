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
