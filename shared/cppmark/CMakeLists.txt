# cppmark — C++ adaptation of cmark-gfm (CommonMark + GFM markdown parser)
# Pure C++23, no external dependencies.

add_library(cppmark STATIC
    src/blocks.cpp
    src/buffer.cpp
    src/chunk.cpp
    src/cmark.cpp
    src/cmark_ctype.cpp
    src/footnotes.cpp
    src/houdini_href_e.cpp
    src/houdini_html_e.cpp
    src/houdini_html_u.cpp
    src/html.cpp
    src/inlines.cpp
    src/iterator.cpp
    src/linked_list.cpp
    src/map.cpp
    src/node.cpp
    src/references.cpp
    src/registry.cpp
    src/render.cpp
    src/scanners.cpp
    src/syntax_extension.cpp
    src/table.cpp
    src/utf8.cpp
)

target_include_directories(cppmark PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(cppmark PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(cppmark PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Precompiled headers — stdlib headers used by most cppmark translation units.
target_precompile_headers(cppmark PRIVATE
    <array>
    <cassert>
    <climits>
    <cstdint>
    <cstdio>
    <cstdlib>
    <cstring>
    <memory>
    <string>
    <string_view>
)
