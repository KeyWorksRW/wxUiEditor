/////////////////////////////////////////////////////////////////////////////
// Purpose:   Simple diff algorithm for comparing text files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "diff.h"

#include <algorithm>

#include "wxue_namespace/wxue_view_vector.h"

auto Diff::SplitLines(std::string_view text) -> std::vector<std::string>
{
    std::vector<std::string> lines;
    std::string_view::size_type start = 0;
    std::string_view::size_type end = 0;

    while (end != std::string_view::npos)
    {
        end = text.find('\n', start);
        if (end == std::string_view::npos)
        {
            // Last line without newline
            if (start < text.size())
            {
                lines.emplace_back(text.substr(start));
            }
        }
        else
        {
            // Include the line without the newline character
            lines.emplace_back(text.substr(start, end - start));
            start = end + 1;
        }
    }

    return lines;
}

auto Diff::ComputeLCS(const std::vector<std::string>& original,
                      const std::vector<std::string>& modified) -> std::vector<std::vector<size_t>>
{
    const size_t orig_size = original.size();
    const size_t mod_size = modified.size();

    // Create LCS table
    std::vector<std::vector<size_t>> lcs(orig_size + 1, std::vector<size_t>(mod_size + 1, 0));

    // Fill LCS table using dynamic programming
    for (size_t orig_idx = 1; orig_idx <= orig_size; ++orig_idx)
    {
        for (size_t mod_idx = 1; mod_idx <= mod_size; ++mod_idx)
        {
            if (original[orig_idx - 1] == modified[mod_idx - 1])
            {
                lcs[orig_idx][mod_idx] = lcs[orig_idx - 1][mod_idx - 1] + 1;
            }
            else
            {
                lcs[orig_idx][mod_idx] =
                    std::max(lcs[orig_idx - 1][mod_idx], lcs[orig_idx][mod_idx - 1]);
            }
        }
    }

    return lcs;
}

void Diff::BuildDiff(const std::vector<std::string>& original,
                     const std::vector<std::string>& modified,
                     const std::vector<std::vector<size_t>>& lcs, DiffResult& result,
                     size_t context_lines)
{
    size_t orig_idx = original.size();
    size_t mod_idx = modified.size();

    std::vector<DiffLine> temp_left;
    std::vector<DiffLine> temp_right;

    // Backtrack through LCS table to build diff
    while (orig_idx > 0 || mod_idx > 0)
    {
        if (orig_idx > 0 && mod_idx > 0 && original[orig_idx - 1] == modified[mod_idx - 1])
        {
            // Lines are the same
            temp_left.push_back(
                { original[orig_idx - 1], DiffType::unchanged, orig_idx - 1, mod_idx - 1 });
            temp_right.push_back(
                { modified[mod_idx - 1], DiffType::unchanged, orig_idx - 1, mod_idx - 1 });
            --orig_idx;
            --mod_idx;
        }
        else if (mod_idx > 0 &&
                 (orig_idx == 0 || lcs[orig_idx][mod_idx - 1] >= lcs[orig_idx - 1][mod_idx]))
        {
            // Line was added
            temp_left.push_back(
                { "", DiffType::added, orig_idx > 0 ? orig_idx - 1 : 0, mod_idx - 1 });
            temp_right.push_back({ modified[mod_idx - 1], DiffType::added,
                                   orig_idx > 0 ? orig_idx - 1 : 0, mod_idx - 1 });
            result.has_differences = true;
            --mod_idx;
        }
        else if (orig_idx > 0)
        {
            // Line was deleted
            temp_left.push_back({ original[orig_idx - 1], DiffType::deleted, orig_idx - 1,
                                  mod_idx > 0 ? mod_idx - 1 : 0 });
            temp_right.push_back(
                { "", DiffType::deleted, orig_idx - 1, mod_idx > 0 ? mod_idx - 1 : 0 });
            result.has_differences = true;
            --orig_idx;
        }
    }

    // Reverse to get correct order
    std::reverse(temp_left.begin(), temp_left.end());
    std::reverse(temp_right.begin(), temp_right.end());

    // Apply context filtering: only include lines near changes
    if (context_lines > 0 && result.has_differences)
    {
        std::vector<bool> include(temp_left.size(), false);

        // Mark lines to include (changes + context)
        for (size_t idx = 0; idx < temp_left.size(); ++idx)
        {
            if (temp_left[idx].type != DiffType::unchanged)
            {
                // Mark the change and surrounding context
                size_t start = (idx >= context_lines) ? idx - context_lines : 0;
                size_t end = std::min(idx + context_lines + 1, temp_left.size());
                for (size_t k = start; k < end; ++k)
                {
                    include[k] = true;
                }
            }
        }

        // Build final result with only included lines
        for (size_t idx = 0; idx < temp_left.size(); ++idx)
        {
            if (include[idx])
            {
                result.left_lines.push_back(temp_left[idx]);
                result.right_lines.push_back(temp_right[idx]);
            }
        }
    }
    else
    {
        // No context filtering, include all lines
        result.left_lines = std::move(temp_left);
        result.right_lines = std::move(temp_right);
    }
}

auto Diff::Compare(std::string_view original, std::string_view modified, size_t context_lines)
    -> DiffResult
{
    DiffResult result;

    auto original_lines = SplitLines(original);
    auto modified_lines = SplitLines(modified);

    if (original_lines.empty() && modified_lines.empty())
    {
        return result;
    }

    auto lcs = ComputeLCS(original_lines, modified_lines);
    BuildDiff(original_lines, modified_lines, lcs, result, context_lines);

    return result;
}

auto Diff::Compare(const wxue::ViewVector& original, const wxue::ViewVector& modified,
                   size_t context_lines) -> DiffResult
{
    DiffResult result;

    if (original.empty() && modified.empty())
    {
        return result;
    }

    // Convert ViewVector to vector<string> for LCS algorithm
    std::vector<std::string> original_lines;
    original_lines.reserve(original.size());
    for (const auto& line: original)
    {
        original_lines.emplace_back(line);
    }

    std::vector<std::string> modified_lines;
    modified_lines.reserve(modified.size());
    for (const auto& line: modified)
    {
        modified_lines.emplace_back(line);
    }

    auto lcs = ComputeLCS(original_lines, modified_lines);
    BuildDiff(original_lines, modified_lines, lcs, result, context_lines);

    return result;
}
