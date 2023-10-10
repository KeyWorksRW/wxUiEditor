cmake_minimum_required(VERSION 3.20)

project(sdi_test LANGUAGES CXX)
project(sdi_test
    LANGUAGES CXX
    VERSION 1.0.0
    DESCRIPTION "wxUiEditor SDI Test"
    HOMEPAGE_URL "https://github.com/KeyWorksRW/wxUiEditor")

####################### Options #######################

option(BUILD_SHARED_LIBS "Build with wxWidgets shared libraries" ON)

if (BUILD_SHARED_LIBS)
    message(NOTICE "Building with wxWidgets shared libraries")
else()
    message(NOTICE "Building with wxWidgets static libraries")
endif()

# This option is designed to make it possible to check changes to a wxWidgets fork, and/or to
# build with the current wxWidgets sources (assuming the wxWidgets fork is in sync).
# Realistically, this is only going to be usable by the maintainers of this project.

option(BUILD_FORK "Builds wxWidgets using wxWidgets/wx_3_3")

option(BUILD_CUSTOM "Builds with custom version of wxWidgets")

if (BUILD_CUSTOM AND BUILD_FORK)
    message(FATAL_ERROR "BUILD_FORK and BUILD_CUSTOM cannot be combined")
endif()

if (BUILD_FORK)
    set(BUILD_ROOT ${BUILD_FORK_DIR} CACHE PATH "${CMAKE_CURRENT_LIST_DIR}/../../../wxWidgets" FORCE)
    message(NOTICE "Building with forked/cloned version of wxWidgets: ${BUILD_ROOT}")
elseif (BUILD_CUSTOM)
    set(BUILD_ROOT ${BUILD_CUSTOM_DIR} CACHE PATH "${CMAKE_CURRENT_LIST_DIR}/../" FORCE)
    message(NOTICE "Building with custom build of wxWidgets")
else()
    set(BUILD_ROOT wxWidgets CACHE PATH "wxWidgets" FORCE)
endif()

option(INTERNAL_DARK_MODE "Enable dark mode (used with wxWidgets 3.3 in _WIN32 builds)" OFF)
option(INTERNAL_DARK_HIGH_CONTRAST "Enable dark mode high contrast (used with wxWidgets 3.3 in _WIN32 builds)" OFF)

####################### Check for Multi-Config Generator #######################

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT isMultiConfig)
    message("\nBecause you are using a single target generator, you MUST specify")
    message("    a \"--config [Debug|Release]\" option with the cmake --build command\n")

    set(allowedBuildTypes Debug Release)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")

    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
    elseif (NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
        message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
    endif()
endif()

####################### General Settings #######################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    # /O1 often results in faster code than /O2 due to CPU caching
    string(REPLACE "/O2" "/O1" cl_optimize ${CMAKE_CXX_FLAGS_RELEASE})
    set(CMAKE_CXX_FLAGS_RELEASE ${cl_optimize} CACHE STRING "C++ Release flags" FORCE)

    # Using /Z7 instead of /Zi to avoid blocking while parallel compilers write to the pdb file.
    # This can considerably speed up build times at the cost of larger object files.
    string(REPLACE "/Zi" "/Z7" z_seven ${CMAKE_CXX_FLAGS_DEBUG})
    set(CMAKE_CXX_FLAGS_DEBUG ${z_seven} CACHE STRING "C++ Debug flags" FORCE)

    # Use static runtime for Release builds to run with Wine without needing to install the dlls
    if (NOT BUILD_SHARED_LIBS)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

add_compile_definitions($<$<CONFIG:Release>:NDEBUG>)

####################### Set wxWidgets location macros #######################

if (BUILD_FORK)
    set (widget_dir ${CMAKE_CURRENT_LIST_DIR}/../../wxWidgets/wx_3_3)
elseif(BUILD_CUSTOM)
    set (widget_dir ${CMAKE_CURRENT_LIST_DIR}/../../wxWidgets/wx_custom)
else()
    set (widget_dir ${CMAKE_CURRENT_LIST_DIR}/../../wxWidgets/wx_3_2)
endif()

if (WIN32)
    if (BUILD_FORK)
        set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/../../wxWidgets/wx_3_3/setup/win)
    elseif(BUILD_CUSTOM)
        set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/../../wxWidgets/wx_custom/setup/win)
    else()
        set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/../../wxWidgets/wx_3_2/setup/win)
    endif()
else()
    if (NOT BUILD_FORK)
        set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/../../wxWidgets/wx_3_3/setup/unix)
    endif()
endif()

message(STATUS "widget_dir: ${widget_dir}")

####################### Libraries and Executables #######################

# Setting CMAKE_MODULE_PATH causes ninja to fail rebuilding until CMake re-generates. Specifying the full path and extension
# means ninja sees this as a normal dependency that didn't change any time one of the files it specifies changes.

include( wxui_code.cmake )  # This will set ${wxue_generated_code} with a list of generated files
include( file_list.cmake )  # This will set ${file_list} with a list of files

add_executable(sdi_test WIN32
    ${file_list}
    ${sdi_generated_code}
)

# This is just used by a github action to confirm that all the source code can be compiled
add_library(check_build STATIC EXCLUDE_FROM_ALL
    ${file_list}
    ${sdi_generated_code}
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(sdi_test PRIVATE WXUSINGDLL)
    target_compile_definitions(check_build PRIVATE WXUSINGDLL)
else()
    # This is built into wxWidgets if a shared library is created
    set(CLib wxCLib)
endif()

if (WIN32)
    target_link_libraries(sdi_test PRIVATE wxWidgets ${CLib} comctl32 Imm32 Shlwapi Version UxTheme)
else()
    target_link_libraries(sdi_test PRIVATE wxWidgets ${CLib})
endif()

if (MSVC)
    # /GL -- combined with the Linker flag /LTCG to perform whole program optimization in Release build
    # /FC -- Full path to source code file in diagnostics
    target_compile_options(sdi_test PRIVATE "$<$<CONFIG:Release>:/GL>" "/FC" "/W4" "/Zc:__cplusplus" "/utf-8")
    target_link_options(sdi_test PRIVATE "$<$<CONFIG:Release>:/LTCG>")

    target_link_options(sdi_test PRIVATE "$<$<CONFIG:Debug>:/natvis:${CMAKE_CURRENT_LIST_DIR}/../../src/wxui.natvis>")

    # Assume the manifest is in the resource file
    target_link_options(sdi_test PRIVATE "/manifest:no")
endif()

target_precompile_headers(sdi_test PRIVATE "cpp/pch.h")
target_precompile_headers(check_build PRIVATE "cpp/pch.h")

if (NOT BUILD_FORK AND NOT BUILD_CUSTOM)
    target_include_directories(sdi_test PRIVATE
        # ${BUILD_ROOT}/include
        ${CMAKE_CURRENT_LIST_DIR}/../../wxWidgets/include
        ${setup_dir}
        ${CMAKE_CURRENT_LIST_DIR}/cpp/
    )
else()
    target_include_directories(sdi_test PRIVATE
        # ${BUILD_ROOT}/include
        ${CMAKE_CURRENT_LIST_DIR}/../../../wxWidgets/include
        ${setup_dir}
        ${CMAKE_CURRENT_LIST_DIR}/cpp/
    )
endif()

target_include_directories(check_build PRIVATE
    # ${BUILD_ROOT}/include
    ${CMAKE_CURRENT_LIST_DIR}/../../../wxWidgets/include
    ${setup_dir}
    ${CMAKE_CURRENT_LIST_DIR}/cpp
)

if (NOT BUILD_FORK AND NOT BUILD_CUSTOM)
    target_link_directories(sdi_test PRIVATE
        $<$<CONFIG:Debug>:${widget_dir}/build/Debug>
        $<$<CONFIG:Release>:${widget_dir}/build/Release>
    )
else()
    # Note that we do not build these libraries -- we assume they are already built in
    # when wxUiEditor was built.
    target_link_directories(sdi_test PRIVATE
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/../../build/stage/lib/Debug>
        $<$<CONFIG:Release>:${CMAKE_CURRENT_LIST_DIR}/../../build/stage/lib/Release>
    )
endif()
