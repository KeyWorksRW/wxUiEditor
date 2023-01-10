cmake_minimum_required(VERSION 3.20)

project(cpptest LANGUAGES CXX)

####################### Options #######################

option(BUILD_SHARED_LIBS "Build with wxWidgets shared libraries" Off)

if (BUILD_SHARED_LIBS)
    message(NOTICE "Building with wxWidgets shared libraries")
endif()

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

set (widget_dir ../wxSnapshot)
set (setup_dir ../wxSnapshot/win)

if (WIN32)
    set(setup_dir ../wxSnapshot/win)
endif()

####################### Libraries and Executables #######################

# Setting CMAKE_MODULE_PATH causes ninja to fail rebuilding until CMake re-generates. Specifying the full path and extension
# means ninja sees this as a normal dependency that didn't change any time one of the files it specifies changes.

include( cpp/wxui_code.cmake )  # This will set ${wxue_generated_code} with a list of generated files
include( cpp/file_list.cmake )  # This will set ${file_list} with a list of files
list(TRANSFORM file_list PREPEND "${file_list_dir}/")

add_executable(cpptest WIN32
    ${setup_dir}
    ${widget_dir}/include
    ${file_list}
    ${wxue_generated_code}
)

# This is just used by a github action to confirm that all the source code can be compiled
add_library(check_build STATIC EXCLUDE_FROM_ALL
    ${setup_dir}
    ${widget_dir}/include
    ${file_list}
    ${wxue_generated_code}
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(cpptest PRIVATE WXUSINGDLL)
    target_compile_definitions(check_build PRIVATE WXUSINGDLL)
else()
    # This is built into wxWidgets if a shared library is created
    set(CLib wxCLib)
endif()

if (WIN32)
    target_link_libraries(cpptest PRIVATE wxWidgets ${CLib} comctl32 Imm32 Shlwapi Version UxTheme)
else()
    target_link_libraries(cpptest PRIVATE wxWidgets ${CLib})
endif()

if (MSVC)
    # /GL -- combined with the Linker flag /LTCG to perform whole program optimization in Release build
    # /FC -- Full path to source code file in diagnostics
    target_compile_options(cpptest PRIVATE "$<$<CONFIG:Release>:/GL>" "/FC" "/W4" "/Zc:__cplusplus" "/utf-8")
    target_link_options(cpptest PRIVATE "$<$<CONFIG:Release>:/LTCG>")

    target_link_options(cpptest PRIVATE "$<$<CONFIG:Debug>:/natvis:${CMAKE_CURRENT_LIST_DIR}/../src/wxui.natvis>")

    # Assume the manifest is in the resource file
    target_link_options(cpptest PRIVATE "/manifest:no")
endif()

target_precompile_headers(cpptest PRIVATE "cpp/pch.h")
target_precompile_headers(check_build PRIVATE "cpp/pch.h")

target_include_directories(cpptest PRIVATE
    ${setup_dir}
    ${widget_dir}/include
    ${CMAKE_CURRENT_LIST_DIR}/cpp/
)

target_include_directories(check_build PRIVATE
    ${setup_dir}
    ${widget_dir}/include
    ${CMAKE_CURRENT_LIST_DIR}/cpp/
)

# Note that we do not build these libraries -- we assume they are already built in
# when wxUiEditor was built.
target_link_directories(cpptest PRIVATE
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/../build/stage/lib/Debug>
    $<$<CONFIG:Release>:${CMAKE_CURRENT_LIST_DIR}/../build/stage/lib/Release>
)
