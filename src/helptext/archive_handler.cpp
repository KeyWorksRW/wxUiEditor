/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles the wxUiEditorData.zip archive
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "archive_handler.h"

#include <cstddef>
#include <span>
#include <string>
#include <vector>

#include <wx/fs_mem.h>
#include <wx/html/htmlwin.h>
#include <wx/string.h>

#include "data/cppmark/include/cppmark_html.h"

#include <glaze/glaze.hpp>

// NOLINTNEXTLINE (global variable) // cppcheck-suppress globalVariable
ArchiveHandler& wxueArchive = ArchiveHandler::get_Instance();

// ###################### OpenArchive #######################

std::expected<void, std::string> ArchiveHandler::OpenArchive(const wxString& zip_path)
{
    if (m_archive)
    {
        return std::unexpected(std::string("Archive is already open"));
    }

    std::expected<std::shared_ptr<DocArchive>, std::string> archive_result =
        OpenDocArchive(zip_path.utf8_string());
    if (!archive_result)
    {
        return std::unexpected(std::move(archive_result.error()));
    }
    m_archive = std::move(*archive_result);

    RegisterMemoryFSHandler();

    return {};
}

// ###################### File access #######################

std::expected<std::string, std::string>
    ArchiveHandler::ReadFile(std::string_view archive_name) const
{
    if (!m_archive)
    {
        return std::unexpected(std::string("Archive is not open"));
    }
    return ReadMarkdown(*m_archive, archive_name);
}

// ###################### Page display #######################

bool ArchiveHandler::DisplayArchivePage(const std::string& archive_name, wxHtmlWindow& html_win)
{
    if (!m_archive)
    {
        return false;
    }

    std::expected<std::string, std::string> markdown_result =
        ReadMarkdown(*m_archive, archive_name);
    if (!markdown_result)
    {
        return false;
    }

    const std::string& markdown = *markdown_result;
    m_current_markdown = markdown;

    const std::string html_text =
        cmark_markdown_to_html(markdown, CMARK_OPT_GITHUB_PRE_LANG | CMARK_OPT_UNSAFE);
    m_current_html_with_ids = AddHeadingIds(html_text);
    html_win.SetPage(wxString::FromUTF8(m_current_html_with_ids));

    m_current_archive_page = archive_name;
    return true;
}

bool ArchiveHandler::DisplayHomePage(wxHtmlWindow& html_win)
{
    return DisplayArchivePage("index.md", html_win);
}

void ArchiveHandler::OnHtmlLink(const wxHtmlLinkInfo& link_info, wxHtmlWindow& html_win)
{
    if (!m_archive)
    {
        return;
    }

    std::string archive_name = link_info.GetHref().utf8_string();

    std::string anchor_name;
    const std::size_t fragment_pos = archive_name.find('#');
    if (fragment_pos != std::string::npos)
    {
        anchor_name = archive_name.substr(fragment_pos + 1);
        archive_name.erase(fragment_pos);
    }

    const std::size_t query_pos = archive_name.find('?');
    if (query_pos != std::string::npos)
    {
        archive_name.erase(query_pos);
    }

    // Same-page anchor navigation
    if (archive_name.empty() || (!anchor_name.empty() && archive_name == m_current_archive_page))
    {
        if (!anchor_name.empty())
        {
            const wxString anchor_href(wxString::FromUTF8("#" + anchor_name));
            std::ignore = html_win.LoadPage(anchor_href);
        }
        return;
    }

    // Different page
    if (DisplayArchivePage(archive_name, html_win) && !anchor_name.empty())
    {
        const wxString anchor_href(wxString::FromUTF8("#" + anchor_name));
        std::ignore = html_win.LoadPage(anchor_href);
    }
}

// ###################### Search #######################

bool ArchiveHandler::LoadSearchIndex()
{
    if (!m_archive)
    {
        return false;
    }

    // Extract doc_map.json
    std::expected<std::string, std::string> doc_map_result =
        ReadMarkdown(*m_archive, "data/doc_map.json");
    if (!doc_map_result)
    {
        return false;
    }

    // Extract search_index.kfts
    std::expected<std::string, std::string> kfts_result =
        ReadMarkdown(*m_archive, "data/search_index.kfts");
    if (!kfts_result)
    {
        return false;
    }

    if (!ParseDocMap(*doc_map_result))
    {
        return false;
    }

    const std::string& kfts_data = *kfts_result;
    const std::span<const char> kfts_chars(kfts_data.data(), kfts_data.size());
    const std::span<const std::byte> kfts_span = std::as_bytes(kfts_chars);

    std::expected<ftsrch::IndexPtr, ftsrch::Error> index_result = ftsrch::OpenIndex(kfts_span);
    if (!index_result)
    {
        return false;
    }
    m_fts_index = std::move(*index_result);

    return true;
}

std::expected<std::vector<ftsrch::QueryResult>, ftsrch::Error>
    ArchiveHandler::Search(std::string_view query) const
{
    if (!m_fts_index)
    {
        return std::unexpected(ftsrch::Error::invalid_argument);
    }
    return ftsrch::Search(*m_fts_index, query);
}

std::expected<std::vector<ftsrch::QueryResult>, ftsrch::Error>
    ArchiveHandler::SearchIncremental(std::string_view partial_query) const
{
    if (!m_fts_index)
    {
        return std::unexpected(ftsrch::Error::invalid_argument);
    }
    return ftsrch::SearchIncremental(*m_fts_index, partial_query);
}

std::string ArchiveHandler::GetArchivePathForDoc(ftsrch::DocId doc_id) const
{
    const auto iter = m_doc_map.find(doc_id);
    if (iter == m_doc_map.end())
    {
        return {};
    }
    return iter->second;
}

// ###################### Private helpers #######################

void ArchiveHandler::RegisterMemoryFSHandler()
{
    static bool handler_registered = false;
    if (!handler_registered)
    {
        wxFileSystem::AddHandler(new wxMemoryFSHandler);
        handler_registered = true;
    }
}

bool ArchiveHandler::ExtractAndRegisterSvgLogo()
{
    std::expected<std::string, std::string> svg_result = ReadMarkdown(*m_archive, "wxlogo.svg");
    if (!svg_result)
    {
        return false;
    }

    const std::string& svg_data = *svg_result;
    wxMemoryFSHandler::AddFileWithMimeType("wxlogo.svg", svg_data.data(), svg_data.size(),
                                           "image/svg+xml");
    return true;
}

bool ArchiveHandler::ParseDocMap(const std::string& json_text)
{
    // doc_map.json format: { "0": "app.md", "1": "bitmap.md", ... }
    std::map<std::string, std::string> raw_map;
    const glz::error_ctx error_ctx = glz::read_json(raw_map, json_text);
    if (error_ctx)
    {
        return false;
    }

    m_doc_map.clear();
    for (const auto& [key_str, path]: raw_map)
    {
        ftsrch::DocId doc_id = 0;
        const auto [end_ptr, errc] =
            std::from_chars(key_str.data(), key_str.data() + key_str.size(), doc_id);
        if (errc != std::errc {})
        {
            continue;
        }
        m_doc_map[doc_id] = path;
    }

    return !m_doc_map.empty();
}

std::string ArchiveHandler::AddHeadingIds(const std::string& html)
{
    std::string result;
    result.reserve(html.size() + 1024);
    std::unordered_map<std::string, int> id_counts;

    std::size_t pos = 0;
    while (pos < html.size())
    {
        // Look for heading opening tags: <h1, <h2, ..., <h6
        if (html[pos] == '<' && pos + 3 < html.size() && html[pos + 1] == 'h' &&
            html[pos + 2] >= '1' && html[pos + 2] <= '6')
        {
            const std::size_t tag_end = html.find('>', pos);
            if (tag_end == std::string::npos)
            {
                result.append(html, pos, html.size() - pos);
                break;
            }

            // Skip if this tag already has an id attribute
            const std::string_view tag_content(html.data() + pos, tag_end - pos);
            if (tag_content.find("id=") != std::string_view::npos)
            {
                result.append(html, pos, tag_end - pos + 1);
                pos = tag_end + 1;
                continue;
            }

            // Find the closing heading tag
            const char heading_level = html[pos + 2];
            const std::string close_tag = std::string("</h") + heading_level + '>';
            const std::size_t close_pos = html.find(close_tag, tag_end);
            if (close_pos == std::string::npos)
            {
                result.append(html, pos, html.size() - pos);
                break;
            }

            // Extract heading text (strip any inner HTML tags for the slug)
            const std::string_view inner_html(html.data() + tag_end + 1, close_pos - tag_end - 1);
            std::string heading_text;
            heading_text.reserve(inner_html.size());
            bool in_tag = false;
            for (const char ch: inner_html)
            {
                if (ch == '<')
                {
                    in_tag = true;
                    continue;
                }
                if (ch == '>')
                {
                    in_tag = false;
                    continue;
                }
                if (!in_tag)
                {
                    heading_text.push_back(ch);
                }
            }

            std::string base_id = SlugifyHeading(heading_text);
            if (base_id.empty())
            {
                base_id = "heading";
            }

            // De-duplicate: append -2, -3, etc.
            std::string unique_id = base_id;
            const int count = ++id_counts[base_id];
            if (count > 1)
            {
                unique_id = base_id + '-' + std::to_string(count);
            }

            // Write: <hN id="unique_id" ...>
            result.append(html, pos, tag_end - pos);
            result.append(" id=\"");
            result.append(unique_id);
            result.push_back('"');
            result.push_back('>');
            pos = tag_end + 1;
        }
        else
        {
            result.push_back(html[pos]);
            ++pos;
        }
    }

    return result;
}

std::string ArchiveHandler::SlugifyHeading(std::string_view text)
{
    std::string slug;
    slug.reserve(text.size());

    for (const char ch: text)
    {
        if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'))
        {
            slug.push_back(ch);
        }
        else if (ch >= 'A' && ch <= 'Z')
        {
            slug.push_back(static_cast<char>(ch + 32));  // to-lower
        }
        else if (!slug.empty() && slug.back() != '-')
        {
            slug.push_back('-');
        }
    }

    // Trim trailing hyphens
    while (!slug.empty() && slug.back() == '-')
    {
        slug.pop_back();
    }

    return slug;
}
