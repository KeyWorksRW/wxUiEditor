/////////////////////////////////////////////////////////////////////////////
// Purpose:   Shared utility declarations
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

// Configuration options for the documentation parser pipeline.
struct ParseOptions
{
    std::filesystem::path input_dir;
    std::filesystem::path output_dir;
    bool verbose { false };
    bool no_fts { false };
    std::optional<std::filesystem::path> single_file;
    std::optional<std::filesystem::path> zip_path;
};

// Run the documentation parser pipeline (scan, parse, write markdown,
// FTS indexing, optional ZIP packaging).  Returns 0 on success, 1 on error.
[[nodiscard]] int RunParser(const ParseOptions& opts);

// Open a parser-produced ZIP archive and verify that every entry can be
// extracted.  Prints per-entry results to stdout.  Returns 0 on success.
[[nodiscard]] int RunZipTest(const std::filesystem::path& zip_path);

// ---------------------------------------------------------------------------
//  Documentation archive reader (parser module)
// ---------------------------------------------------------------------------
//
//  The GUI keeps a DocArchive open for the lifetime of the loaded ZIP and
//  asks it for markdown content by archive-relative filename. This is the
//  starting-point interface; it will grow as the viewer matures.

class DocArchive;

// Open a .zip archive produced by the parser. On failure the unexpected
// value contains a human-readable error message (including archive details).
[[nodiscard]] std::expected<std::shared_ptr<DocArchive>, std::string>
    OpenDocArchive(const std::filesystem::path& zip_path);

// Read a markdown file from an open DocArchive by its archive-relative
// name (for example, "docs/event.md"). On failure the unexpected value
// describes what went wrong (entry not found, decompression error, etc.).
[[nodiscard]] std::expected<std::string, std::string> ReadMarkdown(const DocArchive& archive,
                                                                   std::string_view archive_name);
