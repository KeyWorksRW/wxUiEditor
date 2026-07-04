# cmake/libdeflate/CMakeLists.txt
#
# Builds libdeflate v1.25 as a static library using our own target definition.
# We bypass libdeflate's own CMakeLists.txt to avoid policy issues (notably
# MSVC_RUNTIME_LIBRARY with CMake <3.15 under CMP0091) and to keep full
# control over compile options.
#
# This file is consumed via add_subdirectory() by both the main wxUiEditor
# project and the tools/build_resources standalone project.  Each will
# independently download the source via FetchContent.

include(FetchContent)

FetchContent_Declare(
    libdeflate
    URL https://github.com/ebiggers/libdeflate/releases/download/v1.25/libdeflate-1.25.tar.gz
    DOWNLOAD_NO_PROGRESS TRUE
    SOURCE_SUBDIR nonexistent
)

# SOURCE_SUBDIR points to a non-existent directory so FetchContent_MakeAvailable
# downloads and extracts the source but does NOT run libdeflate's own
# CMakeLists.txt (whose cmake_minimum_required(VERSION 3.10) would reset
# MSVC runtime library policy, breaking the build on CodeQL).
FetchContent_MakeAvailable(libdeflate)

# ── Source file list ────────────────────────────────────────────────
# Derived from libdeflate's own CMakeLists.txt with these options:
#   COMPRESSION_SUPPORT=ON   DECOMPRESSION_SUPPORT=ON
#   ZLIB_SUPPORT=OFF         GZIP_SUPPORT=ON
#   BUILD_SHARED_LIB=OFF     BUILD_TESTS=OFF      BUILD_GZIP=OFF

set(LIBDEFLATE_CORE_SOURCES
    common_defs.h
    libdeflate.h
    lib/arm/cpu_features.c
    lib/arm/cpu_features.h
    lib/cpu_features_common.h
    lib/deflate_constants.h
    lib/lib_common.h
    lib/utils.c
    lib/x86/cpu_features.c
    lib/x86/cpu_features.h
)

set(LIBDEFLATE_COMPRESS_SOURCES
    lib/arm/matchfinder_impl.h
    lib/bt_matchfinder.h
    lib/deflate_compress.c
    lib/deflate_compress.h
    lib/hc_matchfinder.h
    lib/ht_matchfinder.h
    lib/matchfinder_common.h
    lib/riscv/matchfinder_impl.h
    lib/x86/matchfinder_impl.h
)

set(LIBDEFLATE_DECOMPRESS_SOURCES
    lib/decompress_template.h
    lib/deflate_decompress.c
    lib/x86/decompress_impl.h
)

set(LIBDEFLATE_GZIP_SOURCES
    lib/arm/crc32_impl.h
    lib/arm/crc32_pmull_helpers.h
    lib/arm/crc32_pmull_wide.h
    lib/crc32.c
    lib/crc32_multipliers.h
    lib/crc32_tables.h
    lib/gzip_constants.h
    lib/x86/crc32_impl.h
    lib/x86/crc32_pclmul_template.h
    lib/gzip_compress.c
    lib/gzip_decompress.c
)

set(LIBDEFLATE_ALL_SOURCES
    ${LIBDEFLATE_CORE_SOURCES}
    ${LIBDEFLATE_COMPRESS_SOURCES}
    ${LIBDEFLATE_DECOMPRESS_SOURCES}
    ${LIBDEFLATE_GZIP_SOURCES}
)

# Prepend source directory to each path
list(TRANSFORM LIBDEFLATE_ALL_SOURCES PREPEND "${libdeflate_SOURCE_DIR}/")

# ── Static library target ───────────────────────────────────────────

add_library(libdeflate_static STATIC ${LIBDEFLATE_ALL_SOURCES})

# Consumers (miniz, docparser, build_resources) get the include path
# transitively — no need for explicit ${libdeflate_SOURCE_DIR} elsewhere.
target_include_directories(libdeflate_static PUBLIC "${libdeflate_SOURCE_DIR}")

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
