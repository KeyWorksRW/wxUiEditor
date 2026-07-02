/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles the wxUiEditorData.zip archive
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <expected>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "data/ftsrch/ftsrch.h"
#include "data/include/utils.h"

class wxHtmlLinkInfo;
class wxHtmlWindow;

// Opens a documentation ZIP archive, provides per-file access, converts
// markdown pages to HTML for display in a wxHtmlWindow, and optionally
// loads a full-text search index for querying.
//
// The caller supplies the wxHtmlWindow pointer at display time so the
// same ArchiveHandler can serve a dialog, a wxPanel, or any other host.
class ArchiveHandler
{
public:
    ArchiveHandler() = default;
    ~ArchiveHandler() = default;

    ArchiveHandler(const ArchiveHandler&) = delete;
    ArchiveHandler& operator=(const ArchiveHandler&) = delete;
    ArchiveHandler(ArchiveHandler&&) = delete;
    ArchiveHandler& operator=(ArchiveHandler&&) = delete;

    static auto get_Instance() -> ArchiveHandler&
    {
        static ArchiveHandler instance;
        return instance;
    }

    bool is_open() const noexcept { return m_archive != nullptr; }

    // ----- Archive lifecycle -----

    // Open a documentation ZIP archive and prepare the in-memory file
    // system for embedded images (SVG logo rendered as PNG).
    [[nodiscard]] std::expected<void, std::string>
        OpenArchive(const std::filesystem::path& zip_path);

    bool IsOpen() const noexcept { return m_archive != nullptr; }

    // ----- File access -----

    // Read a raw file from the archive by its archive-relative name.
    [[nodiscard]] std::expected<std::string, std::string>
        ReadFile(std::string_view archive_name) const;

    // ----- Page display -----

    // Read a markdown page from the archive, convert it to HTML with
    // heading anchors, and set it on the given wxHtmlWindow.
    [[nodiscard]] bool DisplayArchivePage(const std::string& archive_name, wxHtmlWindow& html_win);

    // Display the home page (index.md).
    [[nodiscard]] bool DisplayHomePage(wxHtmlWindow& html_win);

    // Handle a link-click event — navigate to another archive page
    // or scroll to an anchor within the current page.
    void OnHtmlLink(const wxHtmlLinkInfo& link_info, wxHtmlWindow& html_win);

    // Current archive page name (empty if none displayed).
    const std::string& GetCurrentPage() const noexcept { return m_current_archive_page; }

    // Raw markdown for the currently displayed page (empty if none displayed).
    const std::string& GetCurrentMarkdown() const noexcept { return m_current_markdown; }

    // HTML (with heading IDs injected) for the currently displayed page.
    const std::string& GetCurrentHtml() const noexcept { return m_current_html_with_ids; }

    // ----- Search -----

    // Load the full-text search index from the archive.
    // Call once after OpenArchive before issuing queries.
    [[nodiscard]] bool LoadSearchIndex();

    // Run a full-text search against the loaded index.
    // Returns results sorted by descending relevance score.
    [[nodiscard]] std::expected<std::vector<ftsrch::QueryResult>, ftsrch::Error>
        Search(std::string_view query) const;

    // Incremental (as-you-type) full-text search.
    // The last token is treated as a prefix.
    [[nodiscard]] std::expected<std::vector<ftsrch::QueryResult>, ftsrch::Error>
        SearchIncremental(std::string_view partial_query) const;

    // Return the archive-relative markdown path for a document ID
    // from search results.
    [[nodiscard]] std::string GetArchivePathForDoc(ftsrch::DocId doc_id) const;

    // Whether the search index has been loaded.
    bool HasSearchIndex() const noexcept { return m_fts_index != nullptr; }

    bool ExtractAndRegisterSvgLogo();

private:
    static void RegisterMemoryFSHandler();
    bool ParseDocMap(const std::string& json_text);

    // Add id attributes to HTML heading tags so anchors work.
    // Returns HTML with heading IDs injected.
    static std::string AddHeadingIds(const std::string& html);

    // slugify "Some Heading Text" → "some-heading-text"
    static std::string SlugifyHeading(std::string_view text);

    std::shared_ptr<DocArchive> m_archive;
    ftsrch::IndexPtr m_fts_index;
    std::unordered_map<ftsrch::DocId, std::string> m_doc_map;

    // Cached state for the currently displayed page
    std::string m_current_archive_page;
    std::string m_current_markdown;
    std::string m_current_html_with_ids;
};

extern ArchiveHandler& wxueArchive;  // NOLINT (global variable) // cppcheck-suppress globalVariable
