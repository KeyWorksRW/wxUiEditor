cmake_minimum_required(VERSION 3.20)

project(QuickTest
    LANGUAGES CXX
    VERSION 1.0.0
    DESCRIPTION "wxUiEditor Quick Test"
    HOMEPAGE_URL "https://github.com/KeyWorksRW/wxUiEditor")

####################### Options #######################

# Shared libs should *never* be used for a Release build!
option(BUILD_SHARED_LIBS "Build with wxWidgets shared libraries" ON)

if (BUILD_SHARED_LIBS)
    message(NOTICE "Building with wxWidgets shared libraries")
else()
    message(NOTICE "Building with wxWidgets static libraries")
endif()

# Fork and Custom assume a repository at the same level as wxUiEditor. Both are
# assumed to be built from wxWidgets 3.3 sources and that they have their own
# custom setup.h files.

option(BUILD_FORK "Builds wxWidgets using wxWidgets/wx_3_3")
option(BUILD_CUSTOM "Builds with custom version of wxWidgets")
option(BUILD_WIDGETS_3_2 "Builds with wxWidgets 3.2")

if (BUILD_WIDGETS_3_2)
    if (BUILD_CUSTOM AND BUILD_FORK)
        message(FATAL_ERROR "BUILD_FORK and BUILD_CUSTOM cannot be combined")
    endif()
endif()

if (BUILD_FORK)
    if (BUILD_CUSTOM OR BUILD_WIDGETS_3_2)
        message(FATAL_ERROR "BUILD_FORK cannot be combined with BUILD_WIDGETS_3_2 or BUILD_CUSTOM")
    endif()
endif()

if (BUILD_CUSTOM)
    if (BUILD_FORK OR BUILD_WIDGETS_3_2)
        message(FATAL_ERROR "BUILD_CUSTOM cannot be combined with BUILD_WIDGETS_3_2 or BUILD_FORK")
    endif()
endif()

if (BUILD_FORK)
    # Assume the forked/cloned repository is at the same level as wxUiEditor
    set(BUILD_ROOT ${BUILD_FORK_DIR} CACHE PATH "${CMAKE_CURRENT_LIST_DIR}/../../../wxWidgets" FORCE)
    message(NOTICE "Building with forked/cloned version of wxWidgets: ${BUILD_ROOT}")
elseif (BUILD_CUSTOM)
    # Assume the original wxSnapshot repository is at the same level as wxUiEditor
    set(BUILD_ROOT ${BUILD_CUSTOM_DIR} CACHE PATH "${CMAKE_CURRENT_LIST_DIR}/../../../wxSnapshot" FORCE)
    message(NOTICE "Building with custom build of wxWidgets")
elseif (BUILD_WIDGETS_3_2)
    set(BUILD_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../wxWidgets")
    message(NOTICE "Building with wxWidgets 3.2")
else()
    set(BUILD_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../wxSnapshot")
    message(NOTICE "Building with wxSnapshot 3.3")
endif()

option(INTERNAL_DARK_MODE "Enable dark mode (used with wxWidgets 3.3 in WIN32 builds)" OFF)

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

set(CMAKE_CXX_STANDARD 17)
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
else()
    # Use the package PkgConfig to detect GTK+ headers/library files
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    include_directories(${GTK3_INCLUDE_DIRS})
    link_directories(${GTK3_LIBRARY_DIRS})
    add_definitions(${GTK3_CFLAGS_OTHER})

    pkg_check_modules(X11 REQUIRED x11)
    include_directories(${X11_INCLUDE_DIRS})
    link_directories(${X11_LIBRARY_DIRS})
    add_definitions(${X11_CFLAGS_OTHER})

    # This should work for gcc and clang (including xcode which is based on clang)
    # -O2 can result in faster code than -O3 due to CPU caching.
    string(REPLACE "-O3" "-O2" cl_optimize "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS_RELEASE ${cl_optimize} CACHE STRING "C++ Release flags" FORCE)
endif()

add_compile_definitions($<$<CONFIG:Release>:NDEBUG>)

####################### Set wxWidgets location macros #######################

if (WIN32)
    if (BUILD_FORK)
        set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/../../wxWidgets/wx_3_3/setup/win)
    elseif(BUILD_CUSTOM)
        set(setup_dir ${BUILD_CUSTOM_DIR}/win)
    elseif(BUILD_WIDGETS_3_2)
        set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/../../wxWidgets/wx_3_2/setup/win)
    else()
        set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/../../wxSnapshot/win)
    endif()
elseif (UNIX)
    if (BUILD_FORK)
        set(setup_dir ${BUILD_FORK_DIR}/unix)
    elseif(BUILD_CUSTOM)
        set(setup_dir ${BUILD_CUSTOM_DIR}/unix)
    else()
        set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/../../wxSnapshot/unix)
    endif()
endif()

message(STATUS "BUILD_ROOT: ${BUILD_ROOT}")
message(STATUS "setup_dir: ${setup_dir}")

####################### Libraries and Executables #######################

# Setting CMAKE_MODULE_PATH causes ninja to fail rebuilding until CMake re-generates. Specifying the full path and extension
# means ninja sees this as a normal dependency that didn't change any time one of the files it specifies changes.

include( cpp/file_list.cmake )  # This will set ${file_list} with a list of files
include( quick_code.cmake )      # Generated by wxUiEditor
list(TRANSFORM file_list PREPEND "${file_list_dir}/")

add_executable(QuickTest WIN32
    ${file_list}
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(QuickTest PRIVATE WXUSINGDLL)
else()
    # This is built into wxWidgets if a shared library is created
    set(CLib wxCLib)
endif()

if (WIN32)
    set(SYSTEM_LIBS comctl32 Imm32 Shlwapi Version UxTheme)
elseif (UNIX)
    set(SYSTEM_LIBS
        ${GTK3_LIBRARIES}
        ${X11_LIBRARIES}
        pthread
        png
        dl
        # use static version so this works on older versions of Linux
        libstdc++.a
    )
endif()

if (BUILD_WIDGETS_3_2)
    target_link_libraries(QuickTest PRIVATE wxWidgets wxCLib ${SYSTEM_LIBS})
else()
    target_link_libraries(QuickTest PRIVATE wxWidgets33 wxCLib ${SYSTEM_LIBS})
endif()

if (MSVC)
    # /GL -- combined with the Linker flag /LTCG to perform whole program optimization in Release build
    # /FC -- Full path to source code file in diagnostics
    target_compile_options(QuickTest PRIVATE "$<$<CONFIG:Release>:/GL>" "/FC" "/W4" "/Zc:__cplusplus" "/utf-8")
    target_link_options(QuickTest PRIVATE "$<$<CONFIG:Release>:/LTCG>")

    target_link_options(QuickTest PRIVATE "$<$<CONFIG:Debug>:/natvis:${CMAKE_CURRENT_LIST_DIR}/../../src/wxui.natvis>")

    # Assume the manifest is in the resource file
    target_link_options(QuickTest PRIVATE "/manifest:no")
endif()

target_precompile_headers(QuickTest PRIVATE "cpp/pch.h")

target_include_directories(QuickTest PRIVATE
    ${BUILD_ROOT}/include
    ${setup_dir}

    ${CMAKE_CURRENT_LIST_DIR}/cpp
)

# Note that we do not build these libraries -- we assume they are already built
# when wxUiEditor was built.

if(BUILD_FORK)
    target_link_directories(QuickTest PRIVATE
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/../../../bld/build/Debug>
        $<$<CONFIG:Release>:${CMAKE_CURRENT_LIST_DIR}/../../../bld/build/Release>
    )
elseif(BUILD_CUSTOM)
    target_link_directories(QuickTest PRIVATE
        $<$<CONFIG:Debug>:${BUILD_CUSTOM_DIR}/build/Debug>
        $<$<CONFIG:Release>:${BUILD_CUSTOM_DIR}/build/Release>
    )
elseif(BUILD_WIDGETS_3_2)
    target_link_directories(QuickTest PRIVATE
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/../../wxWidgets/wx_3_2/build/Debug>
        $<$<CONFIG:Release>:${BUILD_CUSTOM_DIR}/../../wxWidgets/wx_3_2/build/Release>
    )
else()
    target_link_directories(QuickTest PRIVATE
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/../../build/stage/lib/Debug>
        $<$<CONFIG:Release>:${CMAKE_CURRENT_LIST_DIR}/../../build/stage/lib/Release>
    )
endif()
