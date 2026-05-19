cmake_minimum_required(VERSION 4.2...4.3)

project(build_resources
    LANGUAGES CXX C
    VERSION 1.0.0.0
    DESCRIPTION "wxUiEditor resource builder"
    HOMEPAGE_URL "https://github.com/KeyWorksRW/wxUiEditor")

####################### Check for Multi-Config Generator #######################

if(NOT CMAKE_GENERATOR_IS_MULTI_CONFIG)
    message("\nBecause you are using a single target generator, you MUST specify")
    message("    a \"--config [Debug|Release]\" option with the cmake --build command\n")

    set(allowedBuildTypes Debug Release)

    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
    elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
        message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
    endif()

    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

####################### libdeflate ######################
# DEFLATE compression/decompression — always Release, size-optimized, no debug symbols

include(FetchContent)
set(FETCHCONTENT_QUIET ON)

FetchContent_Declare(
    libdeflate
    URL https://github.com/ebiggers/libdeflate/releases/download/v1.25/libdeflate-1.25.tar.gz
    DOWNLOAD_NO_PROGRESS TRUE
)

set(LIBDEFLATE_BUILD_SHARED_LIB OFF)
set(LIBDEFLATE_BUILD_TESTS OFF)
set(LIBDEFLATE_BUILD_GZIP OFF)
set(LIBDEFLATE_COMPRESSION_SUPPORT ON)
set(LIBDEFLATE_DECOMPRESSION_SUPPORT ON)
set(LIBDEFLATE_ZLIB_SUPPORT OFF)
set(LIBDEFLATE_GZIP_SUPPORT ON)

FetchContent_MakeAvailable(libdeflate)

# Always optimize for size, never debug, regardless of parent build config
if(MSVC)
    target_compile_options(libdeflate_static PRIVATE /O1)
else()
    target_compile_options(libdeflate_static PRIVATE -Os)
endif()
target_compile_definitions(libdeflate_static PRIVATE NDEBUG)

# Match the parent project's CRT (MD/MDd)
set_target_properties(libdeflate_static PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<CONFIG:Debug>DebugDLL"
)

# ##################### miniz ######################
# ZIP archive reading/writing — always Release, size-optimized, no debug symbols
FetchContent_Declare(
    miniz
    URL https://github.com/richgel999/miniz/archive/refs/tags/3.1.1.zip
    DOWNLOAD_NO_PROGRESS TRUE
)

FetchContent_MakeAvailable(miniz)

# Use libdeflate's CRC-32 instead of miniz's built-in
set_target_properties(miniz PROPERTIES POSITION_INDEPENDENT_CODE ON)
# Don't define zlib-compatible static inline stubs — we use mz_ names directly
target_compile_definitions(miniz INTERFACE MINIZ_NO_ZLIB_COMPATIBLE_NAMES)
target_compile_definitions(miniz PRIVATE USE_EXTERNAL_MZCRC)
target_link_libraries(miniz PRIVATE libdeflate_static)
target_sources(miniz PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../shared/miniz_crc32_wrapper.c)

# Always optimize for size, never debug
if(MSVC)
    target_compile_options(miniz PRIVATE /O1)
else()
    target_compile_options(miniz PRIVATE -Os)
endif()
target_compile_definitions(miniz PRIVATE NDEBUG)

# Match the parent project's CRT
set_target_properties(miniz PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<CONFIG:Debug>DebugDLL"
)

# ###################### Snowball stemmer #######################
# Snowball C stemming library — word normalization for full-text search
FetchContent_Declare(
    snowball
    URL https://snowballstem.org/dist/libstemmer_c-2.2.0.tar.gz
)
FetchContent_MakeAvailable(snowball)

file(GLOB SNOWBALL_RUNTIME_SOURCES "${snowball_SOURCE_DIR}/runtime/*.c")
file(GLOB SNOWBALL_LIBSTEMMER_SOURCES "${snowball_SOURCE_DIR}/libstemmer/libstemmer.c")
file(GLOB SNOWBALL_SRC_C_SOURCES "${snowball_SOURCE_DIR}/src_c/*.c")

add_library(snowball_stemmer STATIC
    ${SNOWBALL_RUNTIME_SOURCES}
    ${SNOWBALL_LIBSTEMMER_SOURCES}
    ${SNOWBALL_SRC_C_SOURCES}
)

target_include_directories(snowball_stemmer PUBLIC "${snowball_SOURCE_DIR}/include")

if(MSVC)
    target_compile_options(snowball_stemmer PRIVATE /W0)
else()
    target_compile_options(snowball_stemmer PRIVATE -w)
endif()

# ###################### Library subdirectories #######################
# ftsrch and cppmark are built as separate static libraries with their own
# precompiled headers.  Neither depends on wxWidgets, so building them here
# (before wxWidgets config) avoids pulling wx headers into their translation
# units, speeding up rebuilds when only wxWidgets changes and vice versa.

add_subdirectory(../../shared/ftsrch  ${CMAKE_BINARY_DIR}/libs/ftsrch)
add_subdirectory(../../shared/cppmark ${CMAKE_BINARY_DIR}/libs/cppmark)
add_subdirectory(../../shared/parser  ${CMAKE_BINARY_DIR}/libs/docparser)

####################### Resource Builder Executable ######################

add_executable(build_resources
    main.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/pugixml/pugixml.cpp
)

target_include_directories(build_resources PRIVATE
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/pugixml
)

target_include_directories(build_resources PRIVATE
    ${libdeflate_SOURCE_DIR}
)

target_link_libraries(build_resources PRIVATE
    docparser
    ftsrch
    cppmark
    snowball_stemmer
    uni-algo::uni-algo
    miniz
    libdeflate_static
)
