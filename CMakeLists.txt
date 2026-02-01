# CMake 3.30 required for FetchContent performance improvements (CMP0168)
cmake_minimum_required(VERSION 3.30)

string( TIMESTAMP YEAR "%y" ) # e.g., "25"
string( TIMESTAMP DOY "%j" ) # e.g., "192"
string( TIMESTAMP PROJECT_YEAR "%Y" )
math( EXPR BUILD_NUM "${YEAR}%100 * 1000 + ${DOY}" )

set( PROJECT_VERSION_MAJOR 1 )
set( PROJECT_VERSION_MINOR 2 )
set( PROJECT_VERSION_PATCH 9 )
set( PROJECT_VERSION_BUILD ${BUILD_NUM} )
set( PROJECT_VERSION_YEAR ${PROJECT_YEAR} )

project( wxUiEditor
    LANGUAGES CXX
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_BUILD}
    DESCRIPTION "wxWidgets UI designer"
    HOMEPAGE_URL "https://github.com/KeyWorksRW/wxUiEditor" )

# ###################### Check for Multi-Config Generator #######################
get_property( isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG )

if( NOT isMultiConfig )
    message( "\nBecause you are using a single target generator, you MUST specify" )
    message( "    a \"--config [Debug|Release]\" option with the cmake --build command\n" )

    set( allowedBuildTypes Debug Release )
    set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}" )

    if( NOT CMAKE_BUILD_TYPE )
        set( CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE )
    elseif( NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes )
        message( FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}" )
    endif()
endif()

# ###################### General Settings #######################
set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_STANDARD_REQUIRED True )
set( CMAKE_CXX_EXTENSIONS OFF )

# ###################### Options #######################
# Only generate compile_commands.json for local development
if ( NOT DEFINED ENV{CI} )
    # creates a compile_commands.json file in the build directory, suitable for clangd
    set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
endif()

# Shared libs should *never* be used for a Release build! For Debug builds, static libraries
# generally reduce link and startup debugger times.
option( BUILD_SHARED_LIBS "Build with wxWidgets shared libraries" OFF )

if( BUILD_SHARED_LIBS )
    message( NOTICE "Building with wxWidgets shared libraries" )
else()
    message( NOTICE "Building with wxWidgets static libraries" )
endif()

option( INTERNAL_BLD_TESTING "Build with internal testing functionality" )

# Setting these will add a matching compiler definition. What that will enable in the code is
# not set -- it's used for a long-running feature that will only be enabled in regular builds
# when the feature is ready for release or testing.
option( INTERNAL_FEATURE1 "Build with Feature #1 enabled" ) # Currently tied to IndexAltName
option( INTERNAL_FEATURE2 "Build with Feature #2 enabled" ) # Currently tied to Data List
option( INTERNAL_FEATURE3 "Build with Feature #3 enabled" ) # unused

# ###################### Version #######################
# Generate C++ header file with version constants for use in source code
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/src/version.h.in
    ${CMAKE_CURRENT_LIST_DIR}/src/version.h
    @ONLY
)

# Generate Windows resource file with version info for executable properties
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/src/version.rc.in
    ${CMAKE_CURRENT_LIST_DIR}/src/version.rc
    @ONLY
)

# ###################### ccache support #######################
find_program( CCACHE_PROGRAM ccache )

if( CCACHE_PROGRAM )
    message( STATUS "Using ccache: ${CCACHE_PROGRAM}" )
    set( CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" )
    set( CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" )

    message( NOTICE "ccache detected. Configure ccache globally for your development environment:" )
    message( NOTICE "  Recommended for all platforms:" )
    message( NOTICE "    ccache --set-config=compression=true" )
    message( NOTICE "    ccache --set-config=max_size=<appropriate for your projects>" )
    message( NOTICE "    ccache --set-config=sloppiness=pch_defines,time_macros,include_file_mtime,include_file_ctime" )

    if( MSVC )
        message( NOTICE "  Required for MSVC:" )

        message( NOTICE "    ccache --set-config=pch_external_checksum=true" )
        message( NOTICE "    ccache --set-config=depend_mode=true" )
        message( NOTICE "    ccache --set-config=\"ignore_options=/Zi /Z7 /ZI /FS /Fd*\"" )
    endif()

    message( NOTICE "  Run 'ccache --show-config' to verify current settings" )
    message( NOTICE "  Run 'ccache -s' to monitor cache usage" )
endif()

if( MSVC )
    # /O1 often results in faster code than /O2 due to CPU caching
    string( REPLACE "/O2" "/O1" cl_optimize "${CMAKE_CXX_FLAGS_RELEASE}" )
    set( CMAKE_CXX_FLAGS_RELEASE ${cl_optimize} CACHE STRING "C++ Release flags" FORCE )

    # Use /Z7 instead of /Zi to embed debug info in object files. Benefits:
    # 1. Avoids blocking when parallel compilers write to PDB files (faster multi-core builds)
    # 2. Required for ccache to cache compilation results
    # Trade-off: Larger object files, but build speed improvement is significant
    string( REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" )
    string( REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}" )
    set( CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} CACHE STRING "C++ Debug flags" FORCE )
    set( CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} CACHE STRING "C Debug flags" FORCE )

    # Use dynamic runtime (/MD for Release, /MDd for Debug)
    set( CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" )

    # Enable AddressSanitizer in Debug builds
    if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
        add_compile_options( /fsanitize=address )
        add_link_options( /fsanitize=address )
        # REVIEW: [Randalphwa - 01-26-2026] Even with MSVC from 2026, incremental linking
        # messes with debugging where you the code you see is not what is actually running as
        # you step through it.
        # add_link_options( /INCREMENTAL:NO /OPT:REF /OPT:ICF )
        add_link_options( /INCREMENTAL:YES /OPT:NOREF /OPT:ICF )
    endif()

    # Increase stack size for large projects with many object files
    string( APPEND CMAKE_EXE_LINKER_FLAGS " /STACK:8388608" ) # 8MB stack
elseif( UNIX AND NOT APPLE )
    # Use the package PkgConfig to detect GTK+ headers/library files
    find_package( PkgConfig REQUIRED )

    pkg_check_modules( GTK3 REQUIRED gtk+-3.0 )
    include_directories( ${GTK3_INCLUDE_DIRS} )
    link_directories( ${GTK3_LIBRARY_DIRS} )
    add_definitions( ${GTK3_CFLAGS_OTHER} )

    pkg_check_modules( X11 REQUIRED x11 )
    include_directories( ${X11_INCLUDE_DIRS} )
    link_directories( ${X11_LIBRARY_DIRS} )
    add_definitions( ${X11_CFLAGS_OTHER} )
endif()

if( NOT MSVC )
    # This should work for gcc and clang (including xcode which is based on clang)
    # -O2 can result in faster code than -O3 due to CPU caching.
    string( REPLACE "-O3" "-O2" cl_optimize "${CMAKE_CXX_FLAGS_RELEASE}" )
    set( CMAKE_CXX_FLAGS_RELEASE ${cl_optimize} CACHE STRING "C++ Release flags" FORCE )
endif()

if( INTERNAL_BLD_TESTING )
    message( NOTICE "Building internal testing version" )

    # In addition to enabling testing-only functionality, this enables ASSERT(), ASSERT_MSG(),
    # and FAIL_MSG() in a Release build.
    add_compile_definitions( INTERNAL_TESTING )
    # include( src/verify/verify.cmake ) # This will set ${verify_files} with a list of source files

    set( wxui_internal_files
        src/internal/msg_logging.cpp
        src/internal/msgframe.cpp
        src/internal/import_panel.cpp
        src/internal/convert_img.cpp
        ${verify_files}
    )
else()
    set( wxui_internal_files
        $<$<CONFIG:Debug>:src/internal/msg_logging.cpp>
        $<$<CONFIG:Debug>:src/internal/msgframe.cpp>
        $<$<CONFIG:Debug>:src/internal/import_panel.cpp>
        $<$<CONFIG:Debug>:src/internal/convert_img.cpp>
    )
endif()

if( INTERNAL_FEATURE1 )
    message( NOTICE "Building with Feature #1 enabled" )
    add_compile_definitions( INTERNAL_FEATURE1 )
endif()

if( INTERNAL_FEATURE2 )
    message( NOTICE "Building with Feature #2 enabled" )
    add_compile_definitions( INTERNAL_FEATURE2 )
endif()

if( INTERNAL_FEATURE3 )
    message( NOTICE "Building with Feature #3 enabled" )
    add_compile_definitions( INTERNAL_FEATURE3 )
endif()

add_compile_definitions( $<$<CONFIG:Release>:NDEBUG> )

# Output binary to root ./bin directory to avoid deletion when build directory is cleaned
if( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin )
endif()

# ###################### Set wxWidgets location macros #######################
if( WIN32 )
    set( wxWidgets_REQUIRED_COMPONENTS base core lunasvg aui html ribbon richtext propgrid webview stc xml xrc )
else()
    set( wxWidgets_REQUIRED_COMPONENTS base core lunasvg aui html ribbon richtext propgrid stc xml xrc )
endif()

include( FetchContent )

# Suppress verbose FetchContent git output (CMake 3.30+)
set(FETCHCONTENT_QUIET ON)

# Set wxWidgets build options BEFORE FetchContent_MakeAvailable
set( wxBUILD_SHARED OFF CACHE BOOL "Build static wx libraries" )
set( wxBUILD_USE_STATIC_RUNTIME OFF CACHE BOOL "Use dynamic runtime library" )
set( wxUSE_UNICODE_UTF8 1 CACHE BOOL "Use UTF-8 internally in wxWidgets" )
set( wxUSE_UTF8_LOCALE_ONLY 1 CACHE BOOL "Use UTF-8 locale only in wxWidgets" )
set( wxUSE_GUI ON CACHE BOOL "This is a GUI application" FORCE )
set( wxUSE_LUNASVG builtin CACHE BOOL "Use LunaSVG for rendering SVG images" FORCE )
set( wxUSE_NANOSVG OFF CACHE BOOL "Use NanoSVG for rendering SVG images" FORCE )
set( wxBUILD_INSTALL OFF CACHE BOOL "Don't build wxWidgets install" )

set( wxUSE_TGA OFF CACHE BOOL "Don't build tga image support" )
set( wxUSE_PCX OFF CACHE BOOL "Don't build pcx image support" )
set( wxUSE_IFF OFF CACHE BOOL "Don't build iff image support" )

# On Windows, the only browser backend that ships with Windows 10 and 11 is IE.
# To use the Edge backend, we would need to install the WebView2 runtime.
set( wxUSE_WEBVIEW_EDGE OFF CACHE BOOL "Use wxWebView Edge backend for web viewing" )

if( WIN32 )
    set( wxUSE_WEBVIEW_IE ON CACHE BOOL "Use wxWebView IE backend for web viewing" )
    set( wxUSE_WEBVIEW ON CACHE BOOL "Use wxWidgets' web viewing classes" )
    set( wxUSE_WEBVIEW_WEBKIT OFF CACHE BOOL "Use wxWebView WebKit for web viewing" )
elseif( APPLE )
    # Apple ships with WebKit, so this should get enabled once it is verified
    # that it works
    # set( wxUSE_WEBVIEW ON CACHE BOOL "Don't use wxWidgets' web viewing classes" )
    # set( wxUSE_WEBVIEW_WEBKIT ON CACHE BOOL "Use wxWebView WebKit for web viewing" )
    set( wxUSE_WEBVIEW OFF CACHE BOOL "Don't use wxWidgets' web viewing classes" )
else()
    # Unix doesn't ship WebKit on all platforms, so we can't use it unless we
    # install it.
    set( wxUSE_WEBVIEW OFF CACHE BOOL "Don't use wxWidgets' web viewing classes" )
endif()

if( APPLE )
    # Force Cocoa toolkit - prevents conflict if GTK is installed (e.g., via Homebrew)
    set( wxBUILD_TOOLKIT "osx_cocoa" CACHE STRING "Use Cocoa toolkit on macOS" FORCE )

    # Ensure the stdlib extension macro is defined for all compilation units (including fetched
    # projects)
    add_definitions( -D__STDC_WANT_LIB_EXT1__=1 )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__STDC_WANT_LIB_EXT1__=1" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_WANT_LIB_EXT1__=1" )

    set( wxUSE_LIBTIFF OFF CACHE BOOL "Don't use libtiff (problems compiling on Mac" )
    set( wxUSE_REGEX OFF CACHE BOOL "Don't use pcre (problems compiling on Mac" )
endif()

message( STATUS "Fetching wxWidgets (this may take awhile) ..." )

FetchContent_Declare(
    wxWidgets
    GIT_REPOSITORY https://github.com/KeyWorksRW/kwxFetch.git
    GIT_TAG origin/dev
    GIT_SHALLOW TRUE
    UPDATE_DISCONNECTED FALSE  # Explicitly enable updates
)

message( NOTICE "Configuring wxWidgets..." )

FetchContent_MakeAvailable( wxWidgets )

message( NOTICE "wxWidgets has been fetched and configured." )
message( STATUS "wxWidgets sources: " ${wxWidgets_SOURCE_DIR} )

# ###################### Libraries and Executables #######################
# Setting CMAKE_MODULE_PATH causes ninja to fail rebuilding until CMake re-generates.
# Specifying the full path and extension means ninja sees this as a normal dependency that
# didn't change any time one of the files it specifies changes.
include( src/wxui/wxui_code.cmake ) # This will set ${wxue_generated_code} with list of generated files
include( src/file_list.cmake ) # This will set ${file_list} with a list of source files
list( TRANSFORM file_list PREPEND "${file_list_dir}/" )

# Note the requirement that --config Debug is used to get the additional debug files
add_executable( wxUiEditor WIN32
    ${file_list}
    ${wxue_generated_code}
    ${wxui_internal_files} # This will be empty unless INTERNAL_BLD_TESTING is set
)

if( BUILD_SHARED_LIBS )
    target_compile_definitions( wxUiEditor PRIVATE WXUSINGDLL )
endif()

if( APPLE )
    set( SYSTEM_LIBS
        pthread
        dl
    )
elseif( UNIX )
    set( SYSTEM_LIBS
        ${GTK3_LIBRARIES}
        ${X11_LIBRARIES}
        pthread
        dl
        png
    )
elseif( WIN32 )
    set( SYSTEM_LIBS comctl32 Imm32 Shlwapi Version UxTheme )
endif()

if( MSVC )
    # /FC -- Full path to source code file in diagnostics
    target_compile_options( wxUiEditor PRIVATE "/FC" "/W4" "/Zc:__cplusplus" "/utf-8" )

    # Manifest is in the resource file
    target_link_options( wxUiEditor PRIVATE "/manifest:no" )
    target_link_directories( wxUiEditor PRIVATE ${WX_LIB_DIR} )

    set_target_properties( wxUiEditor PROPERTIES
        VS_DEBUGGER_NATIVE_VISUALIZER "${CMAKE_CURRENT_LIST_DIR}/src/wxui.natvis"
    )
elseif( APPLE )
    target_compile_definitions( wxUiEditor PRIVATE __WXOSX_COCOA__ )
    target_link_directories( wxUiEditor PRIVATE ${WX_LIB_DIR} )
elseif( UNIX )
    target_compile_definitions( wxUiEditor PRIVATE __WXGTK__ )
    target_link_directories( wxUiEditor PRIVATE ${WX_LIB_DIR} ${GTK3_LIBRARY_DIRS} )
endif()

target_precompile_headers( wxUiEditor PRIVATE "src/pch.h" )

target_include_directories( wxUiEditor PRIVATE
    ${wxWidgets_SOURCE_DIR}/include
)

if( WIN32 )
    target_include_directories( wxUiEditor PRIVATE
        ${wxWidgets_BUILD_DIR}/lib/vc_x64_lib/mswu
    )
elseif( APPLE )
    target_include_directories( wxUiEditor PRIVATE
        ${wxWidgets_SOURCE_DIR}/include
        ${wxWidgets_BUILD_DIR}/lib/osx_cocoa-unicode-static
    )
elseif( UNIX )
    target_include_directories( wxUiEditor PRIVATE
        ${wxWidgets_SOURCE_DIR}/include
        ${wxWidgets_BUILD_DIR}/lib/gtk3_unixu-3.3/include
    )
else()
    message( FATAL_ERROR "Unsupported platform: Only WIN32, APPLE, or UNIX are supported." )
endif()

target_include_directories( wxUiEditor PRIVATE

    ${CMAKE_CURRENT_LIST_DIR}/src/wxue_namespace
    ${CMAKE_CURRENT_LIST_DIR}/src/
    ${CMAKE_CURRENT_LIST_DIR}/src/ttwx
    ${CMAKE_CURRENT_LIST_DIR}/src/tt
    ${CMAKE_CURRENT_LIST_DIR}/src/nodes
    ${CMAKE_CURRENT_LIST_DIR}/src/generate
    ${CMAKE_CURRENT_LIST_DIR}/src/generate/writers
    ${CMAKE_CURRENT_LIST_DIR}/src/mockup
    ${CMAKE_CURRENT_LIST_DIR}/src/project
    ${CMAKE_CURRENT_LIST_DIR}/src/utils
    ${CMAKE_CURRENT_LIST_DIR}/src/customprops
    ${CMAKE_CURRENT_LIST_DIR}/src/ui
    ${CMAKE_CURRENT_LIST_DIR}/src/wxui

    ${CMAKE_CURRENT_LIST_DIR}/frozen/include
    ${CMAKE_CURRENT_LIST_DIR}/src/pugixml
)

foreach( component ${wxWidgets_REQUIRED_COMPONENTS} )
    list( APPEND wxWidgets_LIBRARIES wx${component} )
endforeach()

target_link_libraries( wxUiEditor PRIVATE ${wxWidgets_LIBRARIES} ${SYSTEM_LIBS} )

# ###################### Packaging Instructions #######################
if( UNIX )
    INSTALL( FILES wxUiEditor.desktop DESTINATION share/applications )
    INSTALL( FILES ${CMAKE_CURRENT_LIST_DIR}/src/art_src/wxUiEditor.svg DESTINATION /usr/share/icons/hicolor/scalable/apps )
endif()

# Note: packing is currently only set for a static wxWidgets library build
set( CPACK_PACKAGE_VENDOR "KeyWorks Software" )
set( CPACK_VERBATIM_VARIABLES YES )
set( CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/EULA.TXT )
set( CPACK_PACKAGE_INSTALL_DIRECTORY "wxUiEditor" )
set( CPACK_PACKAGE_EXECUTABLES "wxUiEditor" "wxUiEditor" )
set( CPACK_CREATE_DESKTOP_LINKS wxUiEditor )

set( CPACK_NSIS_MUI_ICON ${CMAKE_CURRENT_LIST_DIR}/src/wxUiEditor.ico )
set( CPACK_NSIS_MUI_UNIICO ${CMAKE_CURRENT_LIST_DIR}/src/wxUiEditor.ico )
set( CPACK_NSIS_INSTALLED_ICON_NAME bin/wxUiEditor.exe )
set( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL YES )
set( CPACK_NSIS_MUI_FINISHPAGE_RUN wxUiEditor.exe )
set( CPACK_NSIS_BRANDING_TEXT "KeyWorks Software" )
set( CPACK_NSIS_BRANDING_TEXT_TRIM_POSITION CENTER )
set( CPACK_NSIS_MODIFY_PATH YES )

set( CPACK_RPM_PACKAGE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/EULA.TXT )
set( CPACK_RPM_PACKAGE_VENDOR "KeyWorks Software" )
set( CPACK_RPM_PACKAGE_GROUP "Development/Tools" )
set( CPACK_RPM_PACKAGE_REQUIRES "libgtk-3.so.0, glibc >= 2.38" )
set( CPACK_RPM_PACKAGE_URL "https://github.com/KeyWorksRW/wxUiEditor" )
set( CPACK_RPM_PACKAGE_DESCRIPTION "wxWidgets UI designer" )
set( CPACK_RPM_PACKAGE_SUMMARY "wxWidgets UI designer" )

# Use the build number to determine if the package is newer than the previous build.
set( CPACK_RPM_PACKAGE_RELEASE ${PROJECT_VERSION_BUILD} )

set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/debian/preinst" )
set( CPACK_DEBIAN_PACKAGE_MAINTAINER "KeyWorks Software <https://github.com/KeyWorksRW>" )
set( CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk-3-0, libc6 (>= 2.31)" )
set( CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/KeyWorksRW/wxUiEditor" )
set( CPACK_DEBIAN_PACKAGE_DESCRIPTION "wxWidgets UI designer" )
set( CPACK_DEBIAN_PACKAGE_ICON ${CMAKE_CURRENT_LIST_DIR}/src/art_src/wxUiEditor.svg )
set( CPACK_DEBIAN_PACKAGE_SUMMARY "wxWidgets UI designer" )

# Use the build number to determine if the package is newer than the previous build.
set( CPACK_DEBIAN_PACKAGE_RELEASE ${PROJECT_VERSION_BUILD} )

# Register .wxui file extension with wxUiEditor
configure_file( "${CMAKE_CURRENT_LIST_DIR}/nsis_options.cmake.in"
    "${PROJECT_BINARY_DIR}/nsis_options.cmake"
    @ONLY )
set( CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/nsis_options.cmake" )

if( WIN32 )
    set( CPACK_GENERATOR ZIP NSIS )
elseif( APPLE )
    set( CPACK_GENERATOR DragNDrop )
    set( CPACK_DMG_VOLUME_NAME "wxUiEditor" )
    set( CPACK_DMG_FORMAT "UDBZ" )  # Compressed DMG
    set( CPACK_DMG_BACKGROUND_IMAGE ${CMAKE_CURRENT_LIST_DIR}/src/art_src/wxUiEditor.svg )
elseif( UNIX )
    set( CPACK_GENERATOR "DEB RPM" )
endif()

# include(GNUInstallDirs)
include( CPack )

install( TARGETS wxUiEditor CONFIGURATIONS Release )

set_property( INSTALL "bin/$<TARGET_FILE_NAME:wxUiEditor>" PROPERTY CPACK_START_MENU_SHORTCUTS "wxUiEditor" )
