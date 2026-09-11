# ###################### Snowball stemmer #######################
# Vendored Snowball C stemming library — word normalization for full-text search.
# Upstream: https://snowballstem.org  (libstemmer_c 2.2.0)
# The sources under this directory are unmodified copies of the official
# libstemmer_c distribution; see README.md and LICENSE.

file(GLOB SNOWBALL_RUNTIME_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/runtime/*.c")
file(GLOB SNOWBALL_LIBSTEMMER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/libstemmer/libstemmer.c")
file(GLOB SNOWBALL_SRC_C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src_c/*.c")

add_library(snowball_stemmer STATIC
    ${SNOWBALL_RUNTIME_SOURCES}
    ${SNOWBALL_LIBSTEMMER_SOURCES}
    ${SNOWBALL_SRC_C_SOURCES}
)

target_include_directories(snowball_stemmer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(MSVC)
    target_compile_options(snowball_stemmer PRIVATE /W0)
else()
    target_compile_options(snowball_stemmer PRIVATE -w)
endif()
