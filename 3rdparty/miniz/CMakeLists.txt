# ###################### miniz #######################
# Vendored miniz — ZIP archive reading/writing plus a zlib-compatible
# DEFLATE/INFLATE implementation.
# Upstream: https://github.com/richgel999/miniz  (3.1.2)
# The sources under this directory are unmodified copies of the upstream
# release; see README.md and LICENSE.
#
# Built as a static library so it is linked directly into its consumers
# (docparser) and the final executable, with no import library required.

include(GenerateExportHeader)

set(MINIZ_SOURCES
    miniz.c
    miniz.h
    miniz_common.h
    miniz_tdef.c
    miniz_tdef.h
    miniz_tinfl.c
    miniz_tinfl.h
    miniz_zip.c
    miniz_zip.h
)

add_library(miniz STATIC ${MINIZ_SOURCES})

# miniz.h includes the generated miniz_export.h, so expose both the source
# directory and the binary directory (where that header is generated).
generate_export_header(miniz)
target_include_directories(miniz PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

# Static build: MINIZ_EXPORT resolves to nothing for consumers.
target_compile_definitions(miniz INTERFACE MINIZ_STATIC_DEFINE)
# Don't define zlib-compatible static inline stubs — we use mz_ names directly.
target_compile_definitions(miniz INTERFACE MINIZ_NO_ZLIB_COMPATIBLE_NAMES)

# Always optimize for size, never debug, regardless of the parent build config.
if(MSVC)
    target_compile_options(miniz PRIVATE /O1)
else()
    target_compile_options(miniz PRIVATE -Os)
endif()
target_compile_definitions(miniz PRIVATE NDEBUG)

set_target_properties(miniz PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)
