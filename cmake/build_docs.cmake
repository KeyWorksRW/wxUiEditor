# cmake/build_docs.cmake
#
# Builds build/archive/wxWidgetsDocs.zip if it doesn't already exist.
#
# Flow:
#   1. If build/archive/wxWidgetsDocs.zip exists, return immediately.
#   2. Create build/archive/ directory.
#   3. Configure and build a Release version of tools/build_resources.
#   4. Run build_resources --parse to generate the zip.
#
# Expected variables from the including scope:
#   CMAKE_CURRENT_SOURCE_DIR  -- root of the wxUiEditor repository
#   CMAKE_COMMAND            -- path to the cmake executable

set(_archive_dir "${CMAKE_CURRENT_SOURCE_DIR}/build/archive")
set(_zip_file "${_archive_dir}/wxWidgetsDocs.zip")

# Step 0: If the zip already exists, we're done.
if(EXISTS "${_zip_file}")
    message(STATUS "wxWidgetsDocs.zip already exists at ${_zip_file}")
    return()
endif()

# Step 1: Create the archive directory.
file(MAKE_DIRECTORY "${_archive_dir}")

# Step 2: Configure and build a Release version of tools/build_resources.
# This is a standalone CMake project -- invoked as a separate process to avoid
# target-name collisions (miniz, snowball_stemmer, etc.) with the parent project.
set(_br_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/tools/build_resources")
set(_br_build_dir "${_br_src_dir}/build")

message(STATUS "Configuring build_resources (Release)...")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        -B "${_br_build_dir}"
        -S "${_br_src_dir}"
        -DCMAKE_BUILD_TYPE=Release
    RESULT_VARIABLE _configure_result
    OUTPUT_VARIABLE _configure_output
    ERROR_VARIABLE _configure_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
)
if(NOT _configure_result EQUAL 0)
    message(FATAL_ERROR
        "Failed to configure build_resources (exit code ${_configure_result}).\n"
        "stdout:\n${_configure_output}\n"
        "stderr:\n${_configure_error}")
endif()

message(STATUS "Building build_resources (Release)...")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        --build "${_br_build_dir}"
        --config Release
    RESULT_VARIABLE _build_result
    OUTPUT_VARIABLE _build_output
    ERROR_VARIABLE _build_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
)
if(NOT _build_result EQUAL 0)
    message(FATAL_ERROR
        "Failed to build build_resources (exit code ${_build_result}).\n"
        "stdout:\n${_build_output}\n"
        "stderr:\n${_build_error}")
endif()

# Locate the built executable. CMakeLists.txt sets CMAKE_RUNTIME_OUTPUT_DIRECTORY
# to ${CMAKE_SOURCE_DIR}/../../bin, so the executable lands in <repo>/bin/ (or
# bin/Release/ for multi-config). Fall back to the build dir for older builds.
set(_br_exe "")
foreach(_candidate
    "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release/build_resources${CMAKE_EXECUTABLE_SUFFIX}"
    "${CMAKE_CURRENT_SOURCE_DIR}/bin/build_resources${CMAKE_EXECUTABLE_SUFFIX}"
    "${_br_build_dir}/Release/build_resources${CMAKE_EXECUTABLE_SUFFIX}"
    "${_br_build_dir}/build_resources${CMAKE_EXECUTABLE_SUFFIX}"
)
    if(EXISTS "${_candidate}")
        set(_br_exe "${_candidate}")
        break()
    endif()
endforeach()
if(_br_exe STREQUAL "")
    message(FATAL_ERROR "Could not locate build_resources executable in ${_br_build_dir}")
endif()

# Step 3: Run build_resources --parse to generate the zip.
set(_interface_wx "${_br_build_dir}/_deps/interface-src/wx")
set(_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/build/docs")

message(STATUS "Generating wxWidgetsDocs.zip via build_resources...")
execute_process(
    COMMAND "${_br_exe}"
        --parse
        --srcdir "${_interface_wx}"
        --outdir "${_output_dir}"
        --zip-path "${_zip_file}"
    RESULT_VARIABLE _parse_result
    OUTPUT_VARIABLE _parse_output
    ERROR_VARIABLE _parse_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
)
if(NOT _parse_result EQUAL 0)
    message(FATAL_ERROR
        "build_resources --parse failed (exit code ${_parse_result}).\n"
        "stdout:\n${_parse_output}\n"
        "stderr:\n${_parse_error}")
endif()

message(STATUS "wxWidgetsDocs.zip generated at ${_zip_file}")
