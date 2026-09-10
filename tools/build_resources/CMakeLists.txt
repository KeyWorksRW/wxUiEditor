cmake_minimum_required(VERSION 4.2...4.3)

project(build_resources
    LANGUAGES CXX C
    VERSION 1.0.0.0
    DESCRIPTION "wxUiEditor resource builder"
    HOMEPAGE_URL "https://github.com/KeyWorksRW/wxUiEditor")

####################### Check for Multi-Config Generator #######################

get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT IS_MULTI_CONFIG)
    set(allowedBuildTypes Debug Release)

    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)  # Default to Debug configuration if not specified
    elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
        message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
    endif()

    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

####################### libdeflate ######################
# DEFLATE compression/decompression — always Release, size-optimized, no debug symbols.
# Built via our own CMakeLists.txt (cmake/libdeflate/) to avoid policy issues
# with libdeflate's cmake_minimum_required(VERSION 3.10).

add_subdirectory(../../cmake/libdeflate ${CMAKE_BINARY_DIR}/libs/libdeflate)

# ##################### miniz ######################
# ZIP archive reading/writing — always Release, size-optimized, no debug symbols.
# Vendored source (see 3rdparty/miniz/README.md); built as a static library.
# The shared miniz target already sets POSITION_INDEPENDENT_CODE,
# MINIZ_NO_ZLIB_COMPATIBLE_NAMES, size optimization, NDEBUG and the MSVC CRT.

add_subdirectory(../../3rdparty/miniz ${CMAKE_BINARY_DIR}/libs/miniz)

# Use libdeflate's CRC-32 instead of miniz's built-in
target_compile_definitions(miniz PRIVATE USE_EXTERNAL_MZCRC)
target_link_libraries(miniz PRIVATE libdeflate_static)
target_sources(miniz PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../src/helptext/data/miniz_crc32_wrapper.c)

# ###################### Snowball stemmer #######################
# Snowball C stemming library — word normalization for full-text search.
# Vendored source (see 3rdparty/Snowball/README.md); built as a static library.

add_subdirectory(../../3rdparty/Snowball ${CMAKE_BINARY_DIR}/libs/Snowball)

# ###################### wxWidgets interface #######################
# These are the wxWidgets header files that parser will convert into markdown
# files while creating a zip archive of the wxWidgets API reference.

FetchContent_Declare(
    interface
    URL https://github.com/KeyWorksRW/kwxFetch/releases/download/latest-interface/kwxFetch-source.tar.xz
    DOWNLOAD_NO_PROGRESS TRUE
    FETCHCONTENT_UPDATES_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(interface)

# ###################### Library subdirectories #######################
# ftsrch and cppmark are built as separate static libraries with their own
# precompiled headers.  Neither depends on wxWidgets, so building them here
# (before wxWidgets config) avoids pulling wx headers into their translation
# units, speeding up rebuilds when only wxWidgets changes and vice versa.

add_subdirectory(../../src/helptext/data/ftsrch ${CMAKE_BINARY_DIR}/libs/ftsrch)
add_subdirectory(../../src/helptext/data/cppmark ${CMAKE_BINARY_DIR}/libs/cppmark)
add_subdirectory(../../src/helptext/data/parser ${CMAKE_BINARY_DIR}/libs/docparser)

# Output binary alongside wxUiEditor in ${CMAKE_SOURCE_DIR}/../bin/<config>/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../../bin)

####################### Resource Builder Executable ######################

add_executable(build_resources
    main.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/pugixml/pugixml.cpp
)

# Debug builds produce build_resourcesD.exe / build_resourcesD
set_target_properties(build_resources PROPERTIES DEBUG_POSTFIX "D")

target_include_directories(build_resources PRIVATE
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/pugixml
)

# libdeflate include path is set PUBLIC on libdeflate_static — no explicit include needed

target_link_libraries(build_resources PRIVATE
    docparser
    ftsrch
    cppmark
    snowball_stemmer
    uni-algo::uni-algo
    miniz
    libdeflate_static
)
