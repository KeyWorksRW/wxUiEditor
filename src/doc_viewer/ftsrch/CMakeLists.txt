# ftsrch — Full-text search library
# Builds independently of wxWidgets, only depends on snowball + uni-algo.

add_library(ftsrch STATIC
    collection.cpp
    compressor.cpp
    dictionary.cpp
    ftsrch.cpp
    index_file.cpp
    phrase_table.cpp
    query.cpp
    stemmer.cpp
    tokenizer.cpp
)

target_include_directories(ftsrch PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ftsrch PUBLIC
    snowball_stemmer
    uni-algo::uni-algo
)

set_target_properties(ftsrch PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Precompiled headers — headers used by most ftsrch translation units.
target_precompile_headers(ftsrch PRIVATE
    <algorithm>
    <bit>
    <cassert>
    <cstddef>
    <cstdint>
    <cstring>
    <expected>
    <filesystem>
    <memory>
    <optional>
    <span>
    <string>
    <string_view>
    <utility>
    <vector>
)
