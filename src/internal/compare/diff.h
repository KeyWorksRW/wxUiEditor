/////////////////////////////////////////////////////////////////////////////
// Purpose:   Simple diff algorithm for comparing text files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace wxue
{
    class ViewVector;
}

enum class DiffType : std::uint8_t
{
    unchanged,
    added,
    deleted,
    modified
};

struct DiffLine
{
    std::string text;
    DiffType type;
    size_t original_line;  // Line number in original file (0-based)
    size_t new_line;       // Line number in new file (0-based)
};

struct DiffResult
{
    std::vector<DiffLine> left_lines;   // Original file lines with context
    std::vector<DiffLine> right_lines;  // New file lines with context
    bool has_differences { false };
};

class Node;

// File difference information for code generation comparison
struct FileDiff
{
    std::string filename;
    std::string original_content;
    std::string new_content;
    DiffResult diff_result;
    Node* form { nullptr };  // The form node that generated this file
};

class Diff
{
public:
    // Compare two strings line-by-line and return the differences
    // context_lines: number of unchanged lines to show before/after changes
    [[nodiscard]] static auto Compare(std::string_view original, std::string_view modified,
                                      size_t context_lines = 3) -> DiffResult;

    // Compare two ViewVectors (already split into lines, line endings normalized)
    [[nodiscard]] static auto Compare(const wxue::ViewVector& original,
                                      const wxue::ViewVector& modified, size_t context_lines = 3)
        -> DiffResult;

private:
    // Split text into lines
    [[nodiscard]] static auto SplitLines(std::string_view text) -> std::vector<std::string>;

    // Compute Longest Common Subsequence using dynamic programming
    [[nodiscard]] static auto ComputeLCS(const std::vector<std::string>& original,
                                         const std::vector<std::string>& modified)
        -> std::vector<std::vector<size_t>>;

    // Build diff result from LCS table
    static void BuildDiff(const std::vector<std::string>& original,
                          const std::vector<std::string>& modified,
                          const std::vector<std::vector<size_t>>& lcs, DiffResult& result,
                          size_t context_lines);
};
