/////////////////////////////////////////////////////////////////////////////
// Purpose:   Find-in-page utilities for markdown/HTML views
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

// Post-process HTML to add id attributes to all <h1>-<h6> tags. The id is
// derived from the heading text (lowercased, non-alphanumeric runs replaced
// with hyphens). Returns the modified HTML and populates heading_ids with
// (original heading text -> generated id) pairs in document order.
std::string AddHeadingIds(std::string_view html,
                          std::vector<std::pair<std::string, std::string>>* heading_ids = nullptr);

// Given heading_ids (as returned by AddHeadingIds) and a 0-based character
// position in markdown, find the heading that contains that position and
// return the corresponding HTML anchor id.  Returns empty string if no
// heading is found.
std::string
    FindSectionForMarkdownPos(std::string_view markdown, std::size_t markdown_pos,
                              std::span<const std::pair<std::string, std::string>> heading_ids);

// Search markdown text for query and return the 0-based character position of
// the first match at or after start_pos (or at 0 if start_pos is npos).
// Returns std::string::npos if not found.
std::size_t FindInMarkdown(std::string_view markdown, std::string_view query,
                           std::size_t start_pos = 0);
