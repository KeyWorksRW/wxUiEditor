cmake_minimum_required(VERSION 3.20)

project(wxUiEditor
    LANGUAGES CXX
    VERSION 1.1.3
    DESCRIPTION "wxWidgets UI designer"
    HOMEPAGE_URL "https://github.com/KeyWorksRW/wxUiEditor")

####################### Options #######################

# Shared libs should *never* be used for a Release build!
option(BUILD_SHARED_LIBS "Build with wxWidgets shared libraries" OFF)

if (BUILD_SHARED_LIBS)
    message(NOTICE "Building with wxWidgets shared libraries")
else()
    message(NOTICE "Building with wxWidgets static libraries")
endif()

option(INTERNAL_BLD_TESTING "Build with internal testing functionality")

# Setting these will add a matching compiler definition. What that will enable in the code is
# not set -- it's used for a long-running feature that will only be enabled in regular builds
# when the feature is ready for release or testing.

option(INTERNAL_FEATURE1 "Build with Feature #1 enabled")
option(INTERNAL_FEATURE2 "Build with Feature #2 enabled")
option(INTERNAL_FEATURE3 "Build with Feature #3 enabled")

# This option is designed to make it possible to check changes to a wxWidgets fork, and/or to
# build with the current wxWidgets sources (assuming the wxWidgets fork is in sync).
# Realistically, this is only going to be usable by the maintainers of this project.

option(BUILD_FORK "Builds wxWidgets using wxWidgets/wx_3_3")

# The following options cannot be combined, are only usable for MSVC compiler, and are only for
# a Release build.

# [Randalphwa - 03-06-2023] Using MSVC Version 19.32.31332 with profiling on changes how
# node->as_checklist_items() works, resulting in invalid C++ code generation for
# wxCheckListBox. As such, the options to profile have been disabled until this can be tested
# again on a newer compiler.

# option(INTERNAL_PGO_GENERATE "Create instrumented build for profiling run (MSVC only)" OFF)
# option(INTERNAL_PGO_USEPROFILE "Create optimized build using profile data (MSVC only)" OFF)

option(BUILD_CUSTOM "Builds with custom version of wxWidgets")

if (BUILD_CUSTOM AND BUILD_FORK)
    message(FATAL_ERROR "You cannot combine BUILD_FORK or BUILD_CUSTOM")
endif()

if (BUILD_FORK)
    set(BUILD_ROOT ${BUILD_FORK_DIR} CACHE PATH "${CMAKE_CURRENT_LIST_DIR}/../wxWidgets" FORCE)
    message(NOTICE "Building with forked/cloned version of wxWidgets")
elseif (BUILD_CUSTOM)
    set(BUILD_ROOT ${BUILD_CUSTOM_DIR} CACHE PATH "${CMAKE_CURRENT_LIST_DIR}/../" FORCE)
    message(NOTICE "Building with custom build of wxWidgets")
else()
    set(BUILD_ROOT wxWidgets CACHE PATH "wxWidgets" FORCE)
    message(NOTICE "Building with wxWidgets 3.3")
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
    string(REPLACE "/O2" "/O1" cl_optimize "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS_RELEASE ${cl_optimize} CACHE STRING "C++ Release flags" FORCE)

    # Using /Z7 instead of /Zi to avoid blocking while parallel compilers write to the pdb
    # file. This can considerably speed up build times at the cost of larger object files.
    string(REPLACE "/Zi" "/Z7" z_seven "${CMAKE_CXX_FLAGS_DEBUG}")
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

if (INTERNAL_BLD_TESTING)
    message(NOTICE "Building internal testing version")

    # In addition to enabling testing-only functionality, this enables ASSERT(), ASSERT_MSG(),
    # and FAIL_MSG() in a Release build.
    add_compile_definitions(INTERNAL_TESTING)
    set (wxui_internal_files
        src/internal/msg_logging.cpp
        src/internal/msgframe.cpp
        src/internal/import_panel.cpp
        src/internal/convert_img.cpp
    )
elseif (INTERNAL_FEATURE1)
    message(NOTICE "Building with Feature #1 enabled")
    add_compile_definitions(INTERNAL_FEATURE1)
elseif (INTERNAL_FEATURE2)
    message(NOTICE "Building with Feature #2 enabled")
    add_compile_definitions(INTERNAL_FEATURE2)
elseif (INTERNAL_FEATURE3)
    message(NOTICE "Building with Feature #3 enabled")
    add_compile_definitions(INTERNAL_FEATURE3)
else()
    set (wxui_internal_files
        $<$<CONFIG:Debug>:src/internal/msg_logging.cpp>
        $<$<CONFIG:Debug>:src/internal/msgframe.cpp>
        $<$<CONFIG:Debug>:src/internal/import_panel.cpp>
        $<$<CONFIG:Debug>:src/internal/convert_img.cpp>
    )
endif()

add_compile_definitions($<$<CONFIG:Release>:NDEBUG>)

set(stageDir ${CMAKE_CURRENT_BINARY_DIR}/stage)

include(GNUInstallDirs)

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${stageDir}/${CMAKE_INSTALL_BINDIR})
endif()

if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${stageDir}/${CMAKE_INSTALL_LIBDIR})
endif()

if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${stageDir}/${CMAKE_INSTALL_LIBDIR})
endif()

####################### Set wxWidgets location macros #######################

if (BUILD_FORK)
    set (widget_dir ${CMAKE_CURRENT_LIST_DIR}/wxWidgets/wx_3_3)
    add_subdirectory(${widget_dir})
elseif(BUILD_CUSTOM)
    set (widget_dir ${CMAKE_CURRENT_LIST_DIR}/wxWidgets/wx_custom)
    add_subdirectory(${widget_dir})
else()
    set (widget_dir ${CMAKE_CURRENT_LIST_DIR}/wxWidgets)
    add_subdirectory(${widget_dir})
endif()

if (WIN32)
    if (BUILD_FORK)
        set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/wxWidgets/wx_3_3/setup/win)
    elseif(BUILD_CUSTOM)
        set(setup_dir ${BUILD_CUSTOM_DIR}/win)
    else()
        set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/wxWidgets/win)
    endif()
elseif (UNIX)
    if (BUILD_FORK)
        set(setup_dir ${BUILD_FORK_DIR}/unix)
    elseif(BUILD_CUSTOM)
        set(setup_dir ${BUILD_CUSTOM_DIR}/unix)
    else()
        set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/wxWidgets/unix)
    endif()
endif()

message(STATUS "widget_dir: ${widget_dir}")
message(STATUS "setup_dir: ${setup_dir}")
message(STATUS "BUILD_ROOT: ${BUILD_ROOT}")

####################### Libraries and Executables #######################

# Setting CMAKE_MODULE_PATH causes ninja to fail rebuilding until CMake re-generates.
# Specifying the full path and extension means ninja sees this as a normal dependency that
# didn't change any time one of the files it specifies changes.

include( src/wxui/wxui_code.cmake )  # This will set ${wxue_generated_code} with list of generated files
include( src/file_list.cmake )       # This will set ${file_list} with a list of source files
list(TRANSFORM file_list PREPEND "${file_list_dir}/")

# Note the requirement that --config Debug is used to get the additional debug files
add_executable(wxUiEditor WIN32
    ${file_list}
    ${wxue_generated_code}
    ${wxui_internal_files}  # This will be empty unless INTERNAL_BLD_TESTING is set
)

# This is just used by a github action to confirm that all the source code can be compiled
add_library(check_build STATIC EXCLUDE_FROM_ALL
    ${file_list}
    ${wxue_generated_code}
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(wxUiEditor PRIVATE WXUSINGDLL)
    target_compile_definitions(check_build PRIVATE WXUSINGDLL)
endif()

if (UNIX)
    set(SYSTEM_LIBS
        ${GTK3_LIBRARIES}
        ${X11_LIBRARIES}
        pthread
        dl
        png
    )
elseif (WIN32)
    set(SYSTEM_LIBS comctl32 Imm32 Shlwapi Version UxTheme)
endif()

target_link_libraries(wxUiEditor PRIVATE wxWidgets33 ${SYSTEM_LIBS} wxCLib)
# target_link_libraries(wxUiEditor PRIVATE wxWidgets33 ${SYSTEM_LIBS} -static-libgcc -static-libstdc++ wxCLib)

if(BUILD_FORK)
    target_link_directories(wxUiEditor PRIVATE
        $<$<CONFIG:Debug>:${widget_dir}/bld/build/Debug>
        $<$<CONFIG:Release>:${widget_dir}/bld/build/Release>
    )
endif()

if (MSVC)
    # /GL -- combined with the Linker flag /LTCG to perform whole program optimization in Release build
    # /FC -- Full path to source code file in diagnostics
    target_compile_options(wxUiEditor PRIVATE "$<$<CONFIG:Release>:/GL>" "/FC" "/W4" "/Zc:__cplusplus" "/utf-8")

    target_link_options(wxUiEditor PRIVATE "$<$<CONFIG:Release>:/LTCG>")
    target_link_options(wxUiEditor PRIVATE "$<$<CONFIG:Debug>:/OPT:REF>")

    # Assume the manifest is in the resource file
    target_link_options(wxUiEditor PRIVATE "/manifest:no")
elseif(UNIX)
    target_compile_options(wxUiEditor PRIVATE "-fPIC")
endif()

target_precompile_headers(wxUiEditor PRIVATE "src/pch.h")
target_precompile_headers(check_build PRIVATE "src/pch.h")

target_include_directories(wxUiEditor PRIVATE
    ${BUILD_ROOT}/include
    ${setup_dir}

    ${CMAKE_CURRENT_LIST_DIR}/frozen/include

    ${CMAKE_CURRENT_LIST_DIR}/src/
    ${CMAKE_CURRENT_LIST_DIR}/src/tt
    ${CMAKE_CURRENT_LIST_DIR}/src/nodes
    ${CMAKE_CURRENT_LIST_DIR}/src/generate
    ${CMAKE_CURRENT_LIST_DIR}/src/project
    ${CMAKE_CURRENT_LIST_DIR}/src/utils
    ${CMAKE_CURRENT_LIST_DIR}/src/customprops
    ${CMAKE_CURRENT_LIST_DIR}/src/ui
    ${CMAKE_CURRENT_LIST_DIR}/src/wxui
    ${CMAKE_CURRENT_LIST_DIR}/pugixml
)

target_include_directories(check_build PRIVATE
    ${BUILD_ROOT}/include
    ${setup_dir}

    ${CMAKE_CURRENT_LIST_DIR}/frozen/include

    ${CMAKE_CURRENT_LIST_DIR}/src/
    ${CMAKE_CURRENT_LIST_DIR}/src/tt
    ${CMAKE_CURRENT_LIST_DIR}/src/nodes
    ${CMAKE_CURRENT_LIST_DIR}/src/generate
    ${CMAKE_CURRENT_LIST_DIR}/src/project
    ${CMAKE_CURRENT_LIST_DIR}/src/utils
    ${CMAKE_CURRENT_LIST_DIR}/src/customprops
    ${CMAKE_CURRENT_LIST_DIR}/src/ui
    ${CMAKE_CURRENT_LIST_DIR}/src/wxui
    ${CMAKE_CURRENT_LIST_DIR}/pugixml
)

####################### Testing #######################

# include( tests/app_tests.cmake )  # This will set ${wxue_generated_code} with list of generated files

####################### Packaging Instructions #######################

if (UNIX)
    INSTALL(FILES wxUiEditor.desktop DESTINATION share/applications)
endif()

# Note: packing is currently only set for a static wxWidgets library build

set(CPACK_PACKAGE_VENDOR "KeyWorks Software")
set(CPACK_VERBATIM_VARIABLES YES)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/EULA.TXT)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "wxUiEditor")
set(CPACK_PACKAGE_EXECUTABLES "wxUiEditor" "wxUiEditor")
set(CPACK_CREATE_DESKTOP_LINKS wxUiEditor)

set(CPACK_NSIS_MUI_ICON ${CMAKE_CURRENT_LIST_DIR}/src/wxUiEditor.ico)
set(CPACK_NSIS_MUI_UNIICO ${CMAKE_CURRENT_LIST_DIR}/src/wxUiEditor.ico)
set(CPACK_NSIS_INSTALLED_ICON_NAME bin/wxUiEditor.exe)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL YES)
set(CPACK_NSIS_MUI_FINISHPAGE_RUN wxUiEditor.exe)
set(CPACK_NSIS_BRANDING_TEXT "KeyWorks Software")
set(CPACK_NSIS_BRANDING_TEXT_TRIM_POSITION CENTER)
set(CPACK_NSIS_MODIFY_PATH YES)

set(CPACK_RPM_PACKAGE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/EULA.TXT)
set(CPACK_RPM_PACKAGE_VENDOR "KeyWorks Software")
set(CPACK_RPM_PACKAGE_GROUP "Development/Tools")
set(CPACK_RPM_PACKAGE_URL "https://github.com/KeyWorksRW/wxUiEditor")
set(CPACK_RPM_PACKAGE_DESCRIPTION "wxWidgets UI designer")
set(CPACK_RPM_PACKAGE_SUMMARY "wxWidgets UI designer")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "KeyWorks Software <https://github.com/KeyWorksRW>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk-3-0, libc6 (>= 2.31)")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/KeyWorksRW/wxUiEditor")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "wxWidgets UI designer")
set(CPACK_DEBIAN_PACKAGE_SUMMARY "wxWidgets UI designer")

# Register .wxui file extension with wxUiEditor

configure_file ("${CMAKE_CURRENT_LIST_DIR}/nsis_options.cmake.in"
                "${PROJECT_BINARY_DIR}/nsis_options.cmake"
                @ONLY)
set (CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/nsis_options.cmake")

if (WIN32)
    set(CPACK_GENERATOR ZIP NSIS)
elseif(UNIX)
    set(CPACK_GENERATOR "DEB RPM")
endif()

# include(GNUInstallDirs)
include(CPack)

install(TARGETS wxUiEditor CONFIGURATIONS Release)

set_property(INSTALL "bin/$<TARGET_FILE_NAME:wxUiEditor>" PROPERTY CPACK_START_MENU_SHORTCUTS "wxUiEditor")
