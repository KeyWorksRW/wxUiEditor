cmake_minimum_required(VERSION 3.20)

project(wxUiEditor
    LANGUAGES CXX
    VERSION 1.0.1
    DESCRIPTION "wxWidgets UI designer"
    HOMEPAGE_URL "https://github.com/KeyWorksRW/wxUiEditor")

####################### Options #######################

option(BUILD_SHARED_LIBS "Build with wxWidgets shared libraries" OFF)

if (BUILD_SHARED_LIBS)
    message(NOTICE "Building with wxWidgets shared libraries")
endif()

option(INTERNAL_BLD_TESTING "Build with internal testing functionality")

# This option is designed to make it possible to check changes to a wxWidgets fork, and/or to build with the current
# wxWidgets sources (assuming the wxWidgets fork is in sync). Realistically, this is only going to be usable by the
# maintainers of this project.
option(INTERNAL_BLD_FORK "Assumes the existance of ../wxWidgets/bld/CMakeLists.txt")

# If this is set, you must build the library yourself. This is required if you build a UNIX version as otherwise wxCLib will
# conflict with Unix dll versions of the sub-libraries such as wxpng.
option(INTERNAL_BLD_WX_CMAKE "When building fork, use wxWidgets build system")

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

    # Using /Z7 instead of /Zi to avoid blocking while parallel compilers write to the pdb file.
    # This can considerably speed up build times at the cost of larger object files.
    string(REPLACE "/Zi" "/Z7" z_seven "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_DEBUG ${z_seven} CACHE STRING "C++ Debug flags" FORCE)

    # Use static runtime for Release builds to run with Wine without needing to install the dlls
    if (NOT BUILD_SHARED_LIBS)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
else()
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

if (NOT INTERNAL_BLD_FORK)
    set (widget_dir ${CMAKE_CURRENT_LIST_DIR}/wxSnapshot)
    set (widget_cmake_dir ${CMAKE_CURRENT_LIST_DIR}/wxSnapshot)

    # This will build wxCLib.lib and wxWidgets.lib
    add_subdirectory(${widget_cmake_dir})
endif()

include( internal.cmake )

if (WIN32)
    if (NOT INTERNAL_BLD_FORK)
        set(setup_dir ${widget_cmake_dir}/win)
        # internal.cmake will have handled the INTERNAL_BLD_FORK settings
    endif()
else()
    if (NOT INTERNAL_BLD_FORK)
        set(setup_dir ${widget_cmake_dir}/unix)
        # internal.cmake will have handled the INTERNAL_BLD_FORK settings
    endif()
endif()

####################### Libraries and Executables #######################

# Instructions for building the ttlib_wx library using this project's precompiled header
include( ttLib_wx/src/ttlib_file_list.cmake )
add_library(ttLib_wx STATIC ${ttlib_file_list})
target_precompile_headers(ttLib_wx PRIVATE "src/pch.h")
target_include_directories(ttLib_wx PRIVATE
    ${widget_dir}/include
    ${setup_dir}
    ${CMAKE_CURRENT_LIST_DIR}/src/
    ${CMAKE_CURRENT_LIST_DIR}/ttLib_wx/src
)

# Setting CMAKE_MODULE_PATH causes ninja to fail rebuilding until CMake re-generates. Specifying the full path and extension
# means ninja sees this as a normal dependency that didn't change any time one of the files it specifies changes.

include( src/wxui/wxui_code.cmake )  # This will set ${wxue_generated_code} with list of generated files
include( src/file_list.cmake )       # This will set ${file_list} with a list of source files
list(TRANSFORM file_list PREPEND "${file_list_dir}/")

# Note the requirement that --config Debug is used to get the additional debug files
add_executable(wxUiEditor WIN32
    ${file_list}
    ${wxue_generated_code}
    ${wxui_internal_files}
)

# This is just used by a github action to confirm that all the source code can be compiled
add_library(check_build STATIC EXCLUDE_FROM_ALL
    ${file_list}
    ${wxue_generated_code}
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(wxUiEditor PRIVATE WXUSINGDLL)
    target_compile_definitions(ttLib_wx PRIVATE WXUSINGDLL)
    target_compile_definitions(check_build PRIVATE WXUSINGDLL)
else()
    # This is built into wxWidgets if a shared library is created
    set(CLib wxCLib)
endif()

if (NOT INTERNAL_BLD_FORK)
    if (WIN32)
        target_link_libraries(wxUiEditor PRIVATE ttLib_wx wxWidgets ${CLib} comctl32 Imm32 Shlwapi Version UxTheme)
    else()
        target_link_libraries(wxUiEditor PRIVATE ttLib_wx wxWidgets ${CLib})
    endif()
else()
    if (WIN32)
        if (INTERNAL_BLD_WX_CMAKE)
            # Both Debug and Release libaries are in the same location
            target_link_directories(wxUiEditor PRIVATE ${widget_lib_dir})
            target_link_libraries(wxUiEditor PRIVATE ttLib_wx ${fork_wxlibraries})
        else()
            target_link_libraries(wxUiEditor PRIVATE ttLib_wx wxWidgets ${CLib})
        endif()
    else()
        target_link_libraries(wxUiEditor PRIVATE ttLib_wx wxWidgets ${CLib})
        target_link_directories(wxUiEditor PRIVATE
            $<$<CONFIG:Debug>:${widget_lib_dir}>
            $<$<CONFIG:Release>:${widget_lib_dir}>
        )
    endif()
endif()

if (MSVC)
    # /GL -- combined with the Linker flag /LTCG to perform whole program optimization in Release build
    # /FC -- Full path to source code file in diagnostics
    target_compile_options(wxUiEditor PRIVATE "$<$<CONFIG:Release>:/GL>" "/FC" "/W4" "/Zc:__cplusplus" "/utf-8")
    target_link_options(wxUiEditor PRIVATE "$<$<CONFIG:Release>:/LTCG>")

    if (PGO_GENERATE)
        if (PGO_USEPROFILE)
            message(FATAL_ERROR "You cannot set both PGO_GENERATE and PGO_USEPROFILE")
        endif()

        message(NOTICE "Creating instrumented build for profiling run")

        target_link_options(wxUiEditor PRIVATE "$<$<CONFIG:Release>:/GENPROFILE:EXACT>")
    endif()

    if (PGO_USEPROFILE)
        message(NOTICE "Creating optimized build using profile data")

        target_link_options(wxUiEditor PRIVATE "$<$<CONFIG:Release>:/USEPROFILE>")
    endif()

    target_link_options(wxUiEditor PRIVATE
        "$<$<CONFIG:Debug>:/natvis:${CMAKE_SOURCE_DIR}/src/wxui.natvis>"
        "$<$<CONFIG:Debug>:/OPT:REF>"
    )

    # Assume the manifest is in the resource file
    target_link_options(wxUiEditor PRIVATE "/manifest:no")
endif()

target_precompile_headers(wxUiEditor PRIVATE "src/pch.h")
target_precompile_headers(check_build PRIVATE "src/pch.h")

target_include_directories(wxUiEditor PRIVATE
    ${widget_dir}/include
    ${setup_dir}

    ${CMAKE_CURRENT_LIST_DIR}/src/
    ${CMAKE_CURRENT_LIST_DIR}/src/nodes
    ${CMAKE_CURRENT_LIST_DIR}/src/generate
    ${CMAKE_CURRENT_LIST_DIR}/src/project
    ${CMAKE_CURRENT_LIST_DIR}/src/utils
    ${CMAKE_CURRENT_LIST_DIR}/src/customprops
    ${CMAKE_CURRENT_LIST_DIR}/src/ui
    ${CMAKE_CURRENT_LIST_DIR}/src/wxui
    ${CMAKE_CURRENT_LIST_DIR}/pugixml
    ${CMAKE_CURRENT_LIST_DIR}/ttLib_wx/src
)

target_include_directories(check_build PRIVATE
    ${widget_dir}/include
    ${setup_dir}

    ${CMAKE_CURRENT_LIST_DIR}/src/
    ${CMAKE_CURRENT_LIST_DIR}/src/nodes
    ${CMAKE_CURRENT_LIST_DIR}/src/generate
    ${CMAKE_CURRENT_LIST_DIR}/src/project
    ${CMAKE_CURRENT_LIST_DIR}/src/utils
    ${CMAKE_CURRENT_LIST_DIR}/src/customprops
    ${CMAKE_CURRENT_LIST_DIR}/src/ui
    ${CMAKE_CURRENT_LIST_DIR}/src/wxui
    ${CMAKE_CURRENT_LIST_DIR}/pugixml
    ${CMAKE_CURRENT_LIST_DIR}/ttLib_wx/src
)

####################### Packaging Instructions #######################

# Note: packing is currently only set for a static wxWidgets library build

set(CPACK_PACKAGE_VENDOR "KeyWorks Software")
set(CPACK_VERBATIM_VARIABLES YES)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/EULA.TXT)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "wxUiEditor")
set(CPACK_PACKAGE_EXECUTABLES "wxUiEditor" "wxUiEditor")

set(CPACK_NSIS_MUI_ICON ${CMAKE_CURRENT_LIST_DIR}/src/wxUiEditor.ico)
set(CPACK_NSIS_MUI_UNIICO ${CMAKE_CURRENT_LIST_DIR}/src/wxUiEditor.ico)
set(CPACK_NSIS_INSTALLED_ICON_NAME bin/wxUiEditor.exe)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL YES)
set(CPACK_NSIS_MUI_FINISHPAGE_RUN wxUiEditor.exe)
set(CPACK_NSIS_BRANDING_TEXT "KeyWorks Software")
set(CPACK_NSIS_BRANDING_TEXT_TRIM_POSITION CENTER)
set(CPACK_NSIS_MODIFY_PATH YES)
set(CPACK_CREATE_DESKTOP_LINKS wxUiEditor)

# Register .wxui file extension with wxUiEditor

configure_file ("${CMAKE_CURRENT_LIST_DIR}/nsis_options.cmake.in"
                "${PROJECT_BINARY_DIR}/nsis_options.cmake"
                @ONLY)
set (CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/nsis_options.cmake")

if (WIN32)
    set(CPACK_GENERATOR ZIP NSIS)
else()
    set(CPACK_GENERATOR TGZ)
endif()

# include(GNUInstallDirs)
include(CPack)

install(TARGETS wxUiEditor CONFIGURATIONS Release)

set_property(INSTALL "bin/$<TARGET_FILE_NAME:wxUiEditor>" PROPERTY CPACK_START_MENU_SHORTCUTS "wxUiEditor")
