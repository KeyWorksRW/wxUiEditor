# Cross-platform CLI tests for wxUiEditor.
#
# Each test re-imports an importer sample and compares the result against the golden
# .wxui file beside it. This exercises the same --verify_import path that
# tests/import_tests/regenerate.py uses, needs no display and no language compiler,
# and produces identical results on every platform. Adding a new golden therefore
# needs no CMake edit -- drop the sample in tests/import_tests/<importer>/ and
# reconfigure.

set(WXUI_EXE $<TARGET_FILE:wxUiEditor>)
set(WXUI_IMPORT_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/import_tests)

# One entry per importer directory and its sample file extension. Keep in sync with
# IMPORTER_DIRS / SAMPLE_EXTENSIONS in tests/import_tests/regenerate.py.
set(WXUI_IMPORT_DIRS winres wxcrafter formbuilder wxsmith wxglade dialogblocks)
set(WXUI_SAMPLE_EXTS rc wxcp fbp wxs wxg pjd)

foreach(importer_dir IN LISTS WXUI_IMPORT_DIRS)
    foreach(sample_ext IN LISTS WXUI_SAMPLE_EXTS)
        file(GLOB sample_files CONFIGURE_DEPENDS
            "${WXUI_IMPORT_TESTS_DIR}/${importer_dir}/*.${sample_ext}")
        foreach(sample_file IN LISTS sample_files)
            get_filename_component(sample_name ${sample_file} NAME_WE)
            set(test_name "verify_import_${importer_dir}_${sample_name}")
            add_test(NAME ${test_name}
                COMMAND ${WXUI_EXE} --verify_import ${sample_file})
            set_tests_properties(${test_name} PROPERTIES
                # The .log written on a mismatch lands beside the sample, so run from
                # the sample directory rather than the build tree.
                WORKING_DIRECTORY "${WXUI_IMPORT_TESTS_DIR}/${importer_dir}"
                LABELS cli
                TIMEOUT 120)
        endforeach()
    endforeach()
endforeach()


# ###################### Unit tests (Catch2 v3) #######################
# The display-free logic (the wxue:: string helpers and the text-diff engine) is compiled into
# the wxui_core static library, which both wxUiEditor and wxui_tests link -- the tests therefore
# exercise the same object files the application ships.
#
# Catch2 is fetched here rather than in the root CMakeLists so that a normal build never
# downloads it. The tag is pinned; GIT_SHALLOW TRUE resolves tags on the CMake version required
# by this project.
include(FetchContent)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(Catch2)

# FetchContent_MakeAvailable() exports the extras directory to this scope, but appending it
# explicitly is the documented, version-safe way to locate Catch.cmake and its helpers.
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# Include directories, the wxWidgets headers and the static wx libraries arrive through
# wxui_core's PUBLIC usage requirements.
add_executable(wxui_tests
    unit/assertion_stub.cpp
    unit/test_diff.cpp
    unit/test_main.cpp
    unit/test_wxue_string.cpp)

target_link_libraries(wxui_tests PRIVATE
    wxui_core
    # Catch2::Catch2 only -- the custom main in tests/unit/test_main.cpp is the entry point.
    Catch2::Catch2
    ${SYSTEM_LIBS})

# catch_discover_tests() has no LABELS argument -- test properties are passed via PROPERTIES, and
# the label is what `ctest -L unit` selects on. Discovery happens at build time, so adding a
# TEST_CASE never requires a CMake edit.
catch_discover_tests(wxui_tests PROPERTIES LABELS unit TIMEOUT 120)
