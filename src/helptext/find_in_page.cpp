/////////////////////////////////////////////////////////////////////////////
// Purpose:   Find-in-page utilities for markdown/HTML views
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "find_in_page.h"

#include <array>
#include <cctype>
#include <span>
#include <string_view>
#include <unordered_map>

// ---------------------------------------------------------------------------
//  Generate an HTML anchor id from heading text.
// ---------------------------------------------------------------------------

static std::string HeadingTextToId(std::string_view heading_text)
{
    // cmark convention: lowercased, punctuation/internal-whitespace → '-',
    // trim leading/trailing hyphens.
    std::string result;
    result.reserve(heading_text.size());

    // Strip Markdown formatting (bold, code spans, links) from the
    // heading text for the purpose of generating the id.
    bool in_code = false;

    for (std::size_t i = 0; i < heading_text.size(); ++i)
    {
        const unsigned char curr_ch = static_cast<unsigned char>(heading_text[i]);

        if (curr_ch == '`')
        {
            in_code = !in_code;
            continue;
        }

        if (in_code)
        {
            // Include code content in the id
            result += static_cast<char>(std::tolower(curr_ch));
            continue;
        }

        // Skip markdown link syntax [...](...) entirely
        if (curr_ch == '[')
        {
            // Skip until matching ']'
            while (i < heading_text.size() && heading_text[i] != ']')
            {
                ++i;
            }
            // Skip the (...)
            if (i < heading_text.size() && heading_text[i] == ']')
            {
                ++i;
                if (i < heading_text.size() && heading_text[i] == '(')
                {
                    while (i < heading_text.size() && heading_text[i] != ')')
                    {
                        ++i;
                    }
                }
            }
            if (i >= heading_text.size())
            {
                break;
            }
            // Continue to process next char — the loop increment will move past it
            if (i > 0)
            {
                --i;  // Let the loop increment handle it
            }
            continue;
        }

        if (curr_ch == '*' || curr_ch == '_')
        {
            // Skip formatting characters
            continue;
        }

        if (std::isalnum(curr_ch))
        {
            result += static_cast<char>(std::tolower(curr_ch));
        }
        else if (curr_ch == ' ' || curr_ch == '\t')
        {
            if (!result.empty() && result.back() != '-')
            {
                result += '-';
            }
        }
        else if (curr_ch == '-' || curr_ch == '.')
        {
            // Keep hyphens and dots in the id
            result += static_cast<char>(curr_ch);
        }
        else
        {
            // Other non-alphanumeric: replace with hyphen
            if (!result.empty() && result.back() != '-')
            {
                result += '-';
            }
        }
    }

    // Trim trailing hyphens
    while (!result.empty() && result.back() == '-')
    {
        result.pop_back();
    }

    if (result.empty())
    {
        result = "section";
    }

    return result;
}

// ---------------------------------------------------------------------------
//  Public: AddHeadingIds
// ---------------------------------------------------------------------------

std::string AddHeadingIds(std::string_view html,
                          std::vector<std::pair<std::string, std::string>>* heading_ids)
{
    std::string result;
    result.reserve(html.size() + 512);  // Slight over-allocate for id attributes

    std::vector<std::pair<std::string, std::string>> local_ids;
    if (heading_ids)
    {
        local_ids.reserve(64);
    }

    // Track generated ids to avoid duplicates (cmark appends -1, -2, etc.)
    std::unordered_map<std::string, int> id_counters;

    constexpr auto heading_tags =
        std::to_array<std::string_view>({ "<h1", "<h2", "<h3", "<h4", "<h5", "<h6" });

    std::size_t html_pos = 0;
    while (html_pos < html.size())
    {
        // Find the next heading tag
        std::size_t tag_start = std::string::npos;
        int heading_level = 0;
        for (int level = 0; level < static_cast<int>(heading_tags.size()); ++level)
        {
            const std::size_t found = html.find(heading_tags[level], html_pos);
            if (found != std::string::npos && (tag_start == std::string::npos || found < tag_start))
            {
                tag_start = found;
                heading_level = level + 1;  // 1-based
            }
        }

        if (tag_start == std::string::npos)
        {
            // No more headings — copy remaining text as-is
            result.append(html.substr(html_pos));
            break;
        }

        // Copy everything up to the heading tag
        result.append(html.substr(html_pos, tag_start - html_pos));

        // Find the end of the opening tag (the '>')
        const std::size_t tag_end = html.find('>', tag_start);
        if (tag_end == std::string::npos)
        {
            // Malformed — copy rest as-is
            result.append(html.substr(tag_start));
            break;
        }

        // Find the closing </hN> tag to extract the heading content
        const std::string closing_tag = "</h" + std::to_string(heading_level) + ">";
        const std::size_t closing_pos = html.find(closing_tag, tag_end);
        if (closing_pos == std::string::npos)
        {
            // No closing tag — just copy the opening tag as-is
            result.append(html.substr(tag_start, tag_end - tag_start + 1));
            html_pos = tag_end + 1;
            continue;
        }

        // Extract heading content (strip any inner tags)
        const std::string_view heading_content(html.data() + tag_end + 1,
                                               closing_pos - tag_end - 1);

        // Strip any HTML tags within heading content
        std::string plain_heading;
        plain_heading.reserve(heading_content.size());
        for (std::size_t i = 0; i < heading_content.size(); ++i)
        {
            if (heading_content[i] == '<')
            {
                while (i < heading_content.size() && heading_content[i] != '>')
                {
                    ++i;
                }
                continue;
            }
            plain_heading += heading_content[i];
        }

        // Generate id from plain heading text
        std::string heading_id = HeadingTextToId(plain_heading);

        // Deduplicate
        auto [count_iter, inserted] = id_counters.try_emplace(heading_id, 0);
        if (!inserted)
        {
            ++count_iter->second;
            heading_id += "-" + std::to_string(count_iter->second);
        }

        if (heading_ids)
        {
            local_ids.emplace_back(plain_heading, heading_id);
        }

        // Copy the opening tag, insert id attribute
        result.append(html.substr(tag_start, tag_end - tag_start));
        result += " id=\"";
        result += heading_id;
        result += "\">";

        // Copy the content and closing tag
        result.append(html.substr(tag_end + 1, closing_pos - tag_end - 1 + closing_tag.size()));

        html_pos = closing_pos + closing_tag.size();
    }

    if (heading_ids)
    {
        *heading_ids = std::move(local_ids);
    }

    return result;
}

// ---------------------------------------------------------------------------
//  Public: FindSectionForMarkdownPos
// ---------------------------------------------------------------------------

std::string
    FindSectionForMarkdownPos(std::string_view markdown, std::size_t markdown_pos,
                              std::span<const std::pair<std::string, std::string>> heading_ids)
{
    if (heading_ids.empty() || markdown_pos >= markdown.size())
    {
        return {};
    }

    // Walk backwards from markdown_pos to find the nearest markdown heading
    const std::string_view slice = markdown.substr(0, markdown_pos);

    // Find the last heading line before markdown_pos
    std::string last_heading_text;

    std::size_t search_pos = 0;
    while (true)
    {
        // Look for lines starting with #
        std::size_t line_start = slice.find('\n', search_pos);
        if (line_start == std::string::npos)
        {
            break;
        }
        ++line_start;  // Skip the \n

        if (line_start >= slice.size())
        {
            break;
        }

        // Check if this line starts a heading
        std::size_t non_space = line_start;
        while (non_space < slice.size() && (slice[non_space] == ' ' || slice[non_space] == '\t'))
        {
            ++non_space;
        }

        if (non_space < slice.size() && slice[non_space] == '#')
        {
            // Found a heading line - extract the text
            std::size_t heading_end = slice.find('\n', non_space);
            if (heading_end == std::string::npos)
            {
                heading_end = slice.size();
            }

            // Skip the # characters and any space
            std::size_t text_start = non_space;
            while (text_start < heading_end && slice[text_start] == '#')
            {
                ++text_start;
            }
            while (text_start < heading_end &&
                   (slice[text_start] == ' ' || slice[text_start] == '\t'))
            {
                ++text_start;
            }

            // Strip trailing {#id} if present (pandoc-style)
            std::string_view text = slice.substr(text_start, heading_end - text_start);

            // Remove trailing {#...}
            if (const std::size_t brace_pos = text.rfind("{#"); brace_pos != std::string::npos)
            {
                text = text.substr(0, brace_pos);
            }

            // Remove trailing spaces
            while (!text.empty() && (text.back() == ' ' || text.back() == '\t'))
            {
                text.remove_suffix(1);
            }

            last_heading_text = text;
        }

        search_pos = line_start;
        if (search_pos >= slice.size())
        {
            break;
        }
    }

    if (last_heading_text.empty())
    {
        // No heading found — use the first heading if any
        if (!heading_ids.empty())
        {
            return heading_ids[0].second;
        }
        return {};
    }

    // Find the heading text in our heading_ids list
    // We need to match by stripping formatting from both sides
    for (const auto& [heading_text, anchor_id]: heading_ids)
    {
        // Compare plain text versions
        std::string simplified_text;
        simplified_text.reserve(heading_text.size());
        for (const char curr_ch: heading_text)
        {
            if (std::isalnum(static_cast<unsigned char>(curr_ch)) || curr_ch == '-' ||
                curr_ch == ' ' || curr_ch == '_')
            {
                simplified_text +=
                    static_cast<char>(std::tolower(static_cast<unsigned char>(curr_ch)));
            }
        }

        std::string simplified_last;
        simplified_last.reserve(last_heading_text.size());
        for (const char curr_ch: last_heading_text)
        {
            if (std::isalnum(static_cast<unsigned char>(curr_ch)) || curr_ch == '-' ||
                curr_ch == ' ' || curr_ch == '_')
            {
                simplified_last +=
                    static_cast<char>(std::tolower(static_cast<unsigned char>(curr_ch)));
            }
        }

        // Remove trailing spaces from simplified
        while (!simplified_text.empty() && simplified_text.back() == ' ')
        {
            simplified_text.pop_back();
        }
        while (!simplified_last.empty() && simplified_last.back() == ' ')
        {
            simplified_last.pop_back();
        }

        if (simplified_text == simplified_last)
        {
            return anchor_id;
        }
    }

    // Fallback: return the first heading's id
    return heading_ids[0].second;
}

// ---------------------------------------------------------------------------
//  Public: FindInMarkdown
// ---------------------------------------------------------------------------

std::size_t FindInMarkdown(std::string_view markdown, std::string_view query,
                           std::size_t start_pos)
{
    if (query.empty() || markdown.empty())
    {
        return std::string::npos;
    }

    if (start_pos >= markdown.size())
    {
        start_pos = 0;
    }

    // Case-sensitive search first
    const std::size_t found = markdown.find(query, start_pos);
    if (found != std::string::npos)
    {
        return found;
    }

    // Fall back to case-insensitive linear scan
    const char first_char =
        static_cast<char>(std::tolower(static_cast<unsigned char>(query.front())));
    for (std::size_t i = start_pos; i < markdown.size(); ++i)
    {
        if (std::tolower(static_cast<unsigned char>(markdown[i])) == first_char)
        {
            // Check if the rest matches
            bool matches = true;
            for (std::size_t j = 0; j < query.size() && (i + j) < markdown.size(); ++j)
            {
                if (std::tolower(static_cast<unsigned char>(markdown[i + j])) !=
                    std::tolower(static_cast<unsigned char>(query[j])))
                {
                    matches = false;
                    break;
                }
            }
            if (matches && (i + query.size()) <= markdown.size())
            {
                return i;
            }
        }
    }

    return std::string::npos;
}
