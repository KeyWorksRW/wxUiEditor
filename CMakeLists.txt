cmake_minimum_required(VERSION 3.20)

project(wxUiEditor
    LANGUAGES CXX
    VERSION 0.9.1
    DESCRIPTION "wxWidgets UI designer"
    HOMEPAGE_URL "https://github.com/KeyWorksRW/wxUiEditor")

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
else()
    # This should work for gcc and clang (including xcode which is based on clang)
    # -O2 can result in faster code than -O3 due to CPU caching.
    string(REPLACE "-O3" "-O2" cl_optimize ${CMAKE_CXX_FLAGS_RELEASE})
    set(CMAKE_CXX_FLAGS_RELEASE ${cl_optimize} CACHE STRING "C++ Release flags" FORCE)
endif()

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

option(INTERNAL_WIDGETS_SOURCES "Build with internal forked wxWidgets sources" )

if (INTERNAL_WIDGETS_SOURCES)
    # This uses a private forked version of wxWidgets containing current sources
    message(NOTICE "Building with internal forked wxWidget libraries")
    set (widget_dir ${CMAKE_CURRENT_LIST_DIR}/../wxWidgets)
    set (widget_cmake_dir ${CMAKE_CURRENT_LIST_DIR}/../wxWidgets/bld)

    # This will build wxCLib.lib and wxWidgets.lib
    add_subdirectory(../wxWidgets/bld ${CMAKE_CURRENT_LIST_DIR}/build/wxWidgets_internal)

    # Add preprocessor definition that can be used to conditionalize code based on whether the
    # internal wxWidgets libraries are being used.
    add_compile_definitions(INTERNAL_WIDGETS)
else()
    # wxSnapshot is dev release (currently 3.1.15)
    set (widget_dir ${CMAKE_CURRENT_LIST_DIR}/wxSnapshot)
    set (widget_cmake_dir ${CMAKE_CURRENT_LIST_DIR}/wxSnapshot)

    # This will build wxCLib.lib and wxWidgets.lib
    add_subdirectory(${widget_cmake_dir})
endif()

add_compile_definitions($<$<CONFIG:Release>:NDEBUG>)

# Comment the following line out for a non-beta Release
add_compile_definitions(BETA)

# Setting CMAKE_MODULE_PATH causes ninja to fail rebuilding until CMake re-generates. Specifying the full path and extension
# means ninja sees this as a normal dependency that didn't change any time one of the files it specifies changes.

include(src/ui/wxui_code.cmake)  # This will set ${wxue_generated_code} with list of generated files
include(src/file_list.cmake)     # This will set ${file_list} with a list of source files

if (INTERNAL_WIDGETS_SOURCES)

    include(src/internal/wxui_internal.cmake)  # This will set ${wxui_internal} with a list of source files

    set (wxui_internal ${wxui_internal}
        ${CMAKE_CURRENT_LIST_DIR}/src/internal/code_compare.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/internal/nodeinfo.cpp
    )

endif()


# Note the requirement that --config Debug is used to get the additional debug files
add_executable(wxUiEditor WIN32
    ${file_list}
    ${wxue_generated_code}
    ${wxui_internal}
)

# This is just used by a github action to confirm that all the source code can be compiled
add_library(check_build STATIC EXCLUDE_FROM_ALL
    ${file_list}
    ${wxue_generated_code}
)

if (WIN32)
    target_link_libraries(wxUiEditor PRIVATE wxWidgets wxCLib comctl32 Imm32 Shlwapi Version UxTheme)
else()
    target_link_libraries(wxUiEditor PRIVATE wxWidgets wxCLib)
endif()

if (MSVC)
    # /GL -- combined with the Linker flag /LTCG to perform whole program optimization in Release build
    # /FC -- Full path to source code file in diagnostics
    target_compile_options(wxUiEditor PRIVATE "$<$<CONFIG:Release>:/GL>" "/FC" "/W4" "/Zc:__cplusplus" "/utf-8")
    target_link_options(wxUiEditor PRIVATE "$<$<CONFIG:Release>:/LTCG>")

    target_link_options(wxUiEditor PRIVATE
        "$<$<CONFIG:Debug>:/natvis:${CMAKE_SOURCE_DIR}/src/wxui.natvis>"
        "$<$<CONFIG:Debug>:/OPT:REF>"
    )

    # Assume the manifest is in the resource file
    target_link_options(wxUiEditor PRIVATE "/manifest:no")
endif()

target_precompile_headers(wxUiEditor PRIVATE "src/pch.h")
target_precompile_headers(check_build PRIVATE "src/pch.h")

if (WIN32)
    set(setup_dir ${widget_cmake_dir}/win)
endif()

target_include_directories(wxUiEditor PRIVATE
    ${widget_dir}/include
    ${setup_dir}
    src/
    src/nodes
    src/generate
    src/utils
    src/customprops
    src/ui
    pugixml
    ttLib/include
)

target_include_directories(check_build PRIVATE
    ${widget_dir}/include
    ${setup_dir}
    src/
    src/nodes
    src/generate
    src/utils
    src/customprops
    src/ui
    pugixml
    ttLib/include
)

# Packaging Instructions

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
set (CPACK_PROJECT_CONFIG_FILE
     "${PROJECT_BINARY_DIR}/nsis_options.cmake")

if (WIN32)
    set(CPACK_GENERATOR ZIP NSIS)
else()
    set(CPACK_GENERATOR TGZ)
endif()

# include(GNUInstallDirs)
include(CPack)

install(TARGETS wxUiEditor
    CONFIGURATIONS Release
)

set_property(INSTALL "bin/$<TARGET_FILE_NAME:wxUiEditor>"
   PROPERTY CPACK_START_MENU_SHORTCUTS "wxUiEditor"
)
