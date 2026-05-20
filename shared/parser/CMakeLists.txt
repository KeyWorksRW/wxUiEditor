# shared/parser/CMakeLists.txt
# 
# Builds the documentation parser as a static library. This library is
# wxWidgets-free — all error reporting goes through a simple console reporter.
# 
# Three source files were migrated from wxViewer and de-wx-ified:
#   run_parser.cpp    — changed wxGetApp() -> parser::Add*()
#   zip_test.cpp      — changed wxGetApp() -> parser::Add*()
#   markdown_writer.cpp — changed wxGetApp() -> parser::Add*()
# 
# The GUI (wxUiEditor proper) can link this same library and provide its
# own reporter implementation if needed.

add_library(docparser STATIC
    # Core parser
    comment_parser.cpp
    decl_parser.cpp
    doc_types.h
    doxy_commands.cpp
    doxy_commands.h
    file_parser.cpp
    file_scanner.cpp
    markdown_writer.cpp
    symbol_table.cpp

    # Pipeline / entry points
    run_parser.cpp

    # Reader / verifier
    zip_reader.cpp
    zip_test.cpp

    # ZIP writer (miniz + libdeflate)
    zip_writer.cpp

    # Console reporter
    parser_reporter.cpp
)

target_include_directories(docparser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(docparser PUBLIC
    ftsrch
    cppmark
    snowball_stemmer
    miniz
    libdeflate_static
)
