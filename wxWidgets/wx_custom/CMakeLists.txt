cmake_minimum_required(VERSION 3.20)

project(wxWidgets
    LANGUAGES
    CXX C
    VERSION 3.3.0.0
    DESCRIPTION "wxWidgets 3.3.0"  # see ../include/wx/version.h
    HOMEPAGE_URL "https://github.com/wxWidgets/wxWidgets"
)

####################### Options #######################

option(BUILD_SHARED_LIBS "Build wxWidgets shared libraries" OFF)

set(BUILD_CUSTOM_DIR "${CMAKE_CURRENT_LIST_DIR}/../../wxSnapshot" CACHE PATH "Root directory of wxCustom UTF8 repository")
if (NOT EXISTS ${BUILD_CUSTOM_DIR})
    message(FATAL_ERROR "Can't find directory in ${BUILD_CUSTOM_DIR}")
endif()

if (BUILD_SHARED_LIBS)
    message(NOTICE "Building custom wxWidgets shared libraries")
else()
    message(NOTICE "Building custom wxWidgets static libraries")
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

set(CMAKE_CXX_STANDARD 17)  # Note that official build is using C++11
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    # /O1 often results in faster code than /O2 due to CPU caching
    string(REPLACE "/O2" "/O1" cl_optimize ${CMAKE_CXX_FLAGS_RELEASE})
    set(CMAKE_CXX_FLAGS_RELEASE ${cl_optimize} CACHE STRING "C++ Release flags" FORCE)

    string(REPLACE "/O2" "/O1" cl_optimize ${CMAKE_C_FLAGS_RELEASE})
    set(CMAKE_C_FLAGS_RELEASE ${cl_optimize} CACHE STRING "C Release flags" FORCE)

    # Using /Z7 instead of /Zi to avoid blocking while parallel compilers write to the pdb file.
    # This can considerably speed up build times at the cost of larger object files.
    string(REPLACE "/Zi" "/Z7" z_seven ${CMAKE_CXX_FLAGS_DEBUG})
    set(CMAKE_CXX_FLAGS_DEBUG ${z_seven} CACHE STRING "C++ Debug flags" FORCE)

    string(REPLACE "/Zi" "/Z7" z_seven ${CMAKE_C_FLAGS_DEBUG})
    set(CMAKE_C_FLAGS_DEBUG ${z_seven} CACHE STRING "C Debug flags" FORCE)

    # Use static runtime for Release builds to run with Wine without needing to install the dlls
    if (NOT BUILD_SHARED_LIBS)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
else()
    # This should work for gcc and clang (including xcode which is based on clang)
    # -O2 can result in faster code than -O3 due to CPU caching.
    string(REPLACE "-O3" "-O2" cl_optimize ${CMAKE_CXX_FLAGS_RELEASE})
    set(CMAKE_CXX_FLAGS_RELEASE ${cl_optimize} CACHE STRING "C++ Release flags" FORCE)
    string(REPLACE "-O3" "-O2" cl_optimize ${CMAKE_C_FLAGS_RELEASE})
    set(CMAKE_C_FLAGS_RELEASE ${cl_optimize} CACHE STRING "C Release flags" FORCE)
endif()

include( wxWidgets.cmake )  # This will set ${common_sources}, ${msw_sources}, ${unix_sources} and ${osx_sources}
include( wxCLib.cmake )     # This will set ${wxCLib_sources} with list of files

list(TRANSFORM common_sources PREPEND "${BUILD_CUSTOM_DIR}/")
list(TRANSFORM msw_sources PREPEND "${BUILD_CUSTOM_DIR}/")
list(TRANSFORM unix_sources PREPEND "${BUILD_CUSTOM_DIR}/")
list(TRANSFORM osx_sources PREPEND "${BUILD_CUSTOM_DIR}/")
list(TRANSFORM wxCLib_sources PREPEND "${BUILD_CUSTOM_DIR}/")

message(STATUS "custom common_sources: ${BUILD_CUSTOM_DIR}/src")

if (WIN32)
    add_library(wxWidgets ${override_sources} ${common_sources} ${msw_sources} )
elseif (APPLE)
    add_library(wxWidgets ${override_sources} ${common_sources} ${osx_sources} )
elseif (UNIX)
    add_library(wxWidgets ${override_sources} ${common_sources} ${unix_sources} )
endif()

add_library(wxCLib STATIC ${wxCLib_sources} )

if (MSVC)
# /GL -- combined with the Linker flag /LTCG allows whole program optimization in Release build
target_compile_options(wxWidgets PRIVATE "$<$<CONFIG:Release>:/GL>")
endif()

if (WIN32)
    target_compile_definitions(wxWidgets PRIVATE
        __WXMSW__
        WIN32
)
endif()

if (BUILD_SHARED_LIBS)
    target_compile_definitions(wxWidgets PRIVATE WXMAKINGDLL)
endif()

target_compile_definitions(wxWidgets PRIVATE
    WXBUILDING
    _CRT_SECURE_NO_DEPRECATE=1
    _CRT_NON_CONFORMING_SWPRINTFS=1
    _SCL_SECURE_NO_WARNINGS=1
    SCI_LEXER
    NO_CXX11_REGEX
    LINK_LEXERS
)

if (WIN32)
    target_compile_definitions(wxCLib PRIVATE
        PCRE2_CODE_UNIT_WIDTH=16
        NEWLINE_DEFAULT=2
        LINK_SIZE=2
        PARENS_NEST_LIMIT=250
        HEAP_LIMIT=20000000
        MATCH_LIMIT=10000000
        MATCH_LIMIT_DEPTH=10000000
        MAX_NAME_COUNT=10000
        MAX_NAME_SIZE=32
    )
    else()
        target_compile_definitions(wxCLib PRIVATE
        PCRE2_CODE_UNIT_WIDTH=8
        NEWLINE_DEFAULT=2
        LINK_SIZE=2
        PARENS_NEST_LIMIT=250
        HEAP_LIMIT=20000000
        MATCH_LIMIT=10000000
        MATCH_LIMIT_DEPTH=10000000
        MAX_NAME_COUNT=10000
        MAX_NAME_SIZE=32
    )
endif()

target_precompile_headers(wxWidgets PRIVATE "${BUILD_CUSTOM_DIR}/include/wx/wxprec.h")

if (WIN32)
    set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/setup/win)
elseif(UNIX)
    set(setup_dir ${CMAKE_CURRENT_LIST_DIR}/setup/unix)
endif()


message(STATUS "custom BUILD_CUSTOM_DIR: ${BUILD_CUSTOM_DIR}")
message(STATUS "custom setup_dir: ${setup_dir}")

target_include_directories(wxWidgets PRIVATE
    ${BUILD_CUSTOM_DIR}/include
    ${setup_dir}
    ${BUILD_CUSTOM_DIR}/src/tiff/libtiff
    ${BUILD_CUSTOM_DIR}/src/jpeg
    ${BUILD_CUSTOM_DIR}/src/png
    ${BUILD_CUSTOM_DIR}/src/zlib
    ${BUILD_CUSTOM_DIR}/3rdparty/pcre/src/wx
    ${BUILD_CUSTOM_DIR}/src/expat/expat/lib

    # Needed for the wx_stc library
    ${BUILD_CUSTOM_DIR}/src/stc/scintilla/include
    ${BUILD_CUSTOM_DIR}/src/stc/lexilla/include
    ${BUILD_CUSTOM_DIR}/src/stc/lexilla/access
    ${BUILD_CUSTOM_DIR}/src/stc/lexilla/lexlib
    ${BUILD_CUSTOM_DIR}/src/stc/scintilla/src
)

target_include_directories(wxCLib PRIVATE
    ${BUILD_CUSTOM_DIR}/include
    ${setup_dir}
    ${BUILD_CUSTOM_DIR}/src/tiff/libtiff
    ${BUILD_CUSTOM_DIR}/src/jpeg
    ${BUILD_CUSTOM_DIR}/src/zlib
    ${BUILD_CUSTOM_DIR}/3rdparty/pcre/src/wx
    ${BUILD_CUSTOM_DIR}/src/expat/expat/lib
)
