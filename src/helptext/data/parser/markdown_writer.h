/////////////////////////////////////////////////////////////////////////////
// Purpose:   Write parsed documentation as Markdown files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "doc_types.h"
#include "symbol_table.h"

#include <filesystem>

namespace docparser
{

    // Convert a class name to its output .md filename, preserving case.
    // Example: "wxButton" -> "wxButton.md"
    [[nodiscard]] std::string ClassNameToFile(const std::string& name);

    // Write one .md file per class found in content, one .md per @page topic, plus
    // one .md for any non-class content (enums, typedefs, defines, free functions)
    // keyed to source_rel_path.
    // 'input_dir' is the parser's scan root; its leaf name is prefixed to
    // source_rel_path to reconstruct the canonical '#include' path (e.g. "wx/foo.h").
    // When empty, the include path falls back to source_rel_path alone.
    // 'non_class_stem' overrides the output stem for the non-class file so that two
    // headers with the same basename never share an output name; empty keeps the
    // source stem.
    // Asserts that source_rel_path has a .h extension.
    // Returns the paths of all generated files, relative to output_dir.
    [[nodiscard]] std::vector<std::filesystem::path> WriteFile(
        const FileContent& content, const SymbolTable& symbols,
        const std::filesystem::path& output_dir, const std::filesystem::path& source_rel_path,
        const std::filesystem::path& input_dir = {}, const std::string& non_class_stem = {});

}  // namespace docparser
