/////////////////////////////////////////////////////////////////////////////
// Purpose:   Reusable doc viewer panel (wxHtmlWindow + index tabs)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-04-2026]

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/frame.h>
#include <wx/fs_mem.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

#include <glaze/glaze.hpp>
#include <utility>

#include "doc_view_panel.h"

#include "archive_handler.h"
#include "find_in_page.h"
#include "inherit_graph.h"  // docparser::InheritGraphNode, RenderInheritanceSvg

// ---------------------------------------------------------------------------
//  Glaze meta — serialize/deserialize InheritEntry by name.
//  Placed here (not in the header) to avoid pulling glaze into every TU.
// ---------------------------------------------------------------------------

template <>
struct glz::meta<InheritEntry>
{
    using Type = InheritEntry;
    // NOLINTNEXTLINE(readability-avoid-auto) — type is detail::Object<glz::tuple<...>>, not
    // writable
    static constexpr auto value =  // NOLINT(readability-avoid-auto) — glz type not expressible
        glz::object("bases", &Type::bases, "derived", &Type::derived);
};

// ---------------------------------------------------------------------------
//  Constructors
// ---------------------------------------------------------------------------

DocViewPanel::DocViewPanel(wxWindow* parent) : DocViewPanelBase(parent)
{
    InitPanel();
}

// ---------------------------------------------------------------------------
//  InitPanel — bind Find toolbar button and Ctrl+F after Create() runs
// ---------------------------------------------------------------------------

void DocViewPanel::InitPanel()
{
    for (int idx = 0; std::cmp_less(idx, m_aui_tool_bar->GetToolCount()); ++idx)
    {
        const wxAuiToolBarItem* const item = m_aui_tool_bar->FindToolByIndex(idx);
        if (item != nullptr)
        {
            const int tool_id = item->GetId();
            if (tool_id != m_home->GetId() && tool_id != m_back->GetId() &&
                tool_id != m_forward->GetId())
            {
                m_find_tool_id = tool_id;
                break;
            }
        }
    }

    // Ctrl+F via CHAR_HOOK — more reliable than an accelerator table with
    // controls that capture keystrokes at a low level (wxHtmlWindow, etc.)
    Bind(wxEVT_CHAR_HOOK,
         [this](wxKeyEvent& key_event)
         {
             if (key_event.ControlDown() && key_event.GetKeyCode() == static_cast<int>('F'))
             {
                 wxCommandEvent dummy;
                 OnFind(dummy);
                 return;
             }
             key_event.Skip();
         });

    // F3 via CHAR_HOOK — advance to the next find match (same rationale as Ctrl+F
    // above: controls that capture keystrokes at a low level, such as wxHtmlWindow).
    Bind(wxEVT_CHAR_HOOK,
         [this](wxKeyEvent& key_event)
         {
             if (key_event.GetKeyCode() == WXK_F3 && !key_event.ControlDown() &&
                 !key_event.AltDown())
             {
                 OnFindNext(key_event);
                 return;
             }
             key_event.Skip();
         });
}

// ---------------------------------------------------------------------------
//  OpenArchive
// ---------------------------------------------------------------------------

bool DocViewPanel::OpenArchive(const wxString& zip_path)
{
    // Reset state for the new archive
    m_inherit_map.clear();
    m_current_graph_fs_name.clear();

    m_classes_items.clear();
    m_events_items.clear();
    m_functions_items.clear();
    m_overviews_items.clear();

    m_classes_listbox->Clear();
    m_events_listbox->Clear();
    m_functions_listbox->Clear();
    m_overviews_listbox->Clear();

    std::expected<void, std::string> open_result = wxueArchive.OpenArchive(zip_path);
    if (!open_result)
    {
        SetStatusMessage(wxString::FromUTF8(open_result.error()));
        return false;
    }

    // Register SVG logo in the memory file system
    std::ignore = wxueArchive.ExtractAndRegisterSvgLogo();

    // Load the full-text search index
    std::ignore = wxueArchive.LoadSearchIndex();

    // Optional: load data/inheritance.json for on-the-fly inheritance graphs.
    if (const std::expected<std::string, std::string> inherit_result =
            wxueArchive.ReadFile("data/inheritance.json");
        inherit_result)
    {
        std::unordered_map<std::string, InheritEntry> parsed;
        const glz::error_ctx parse_err = glz::read_json(parsed, *inherit_result);
        if (!parse_err)
        {
            m_inherit_map = std::move(parsed);
        }
    }

    // Populate the classes tab immediately (default tab shown on startup).
    PopulateIndexListbox("data/classes.md", m_classes_listbox, m_classes_textctrl, wxT("wx"),
                         m_classes_items);

    // Display home page
    std::ignore = wxueArchive.DisplayHomePage(*m_html_win);
    RecordNavigation("index.md");

    m_archive_open = true;
    return true;
}

bool DocViewPanel::IsArchiveOpen() const
{
    return m_archive_open;
}

void DocViewPanel::NavigateHome()
{
    if (wxueArchive.is_open())
    {
        RecordNavigation("index.md");
        std::ignore = wxueArchive.DisplayHomePage(*m_html_win);
        SetStatusMessage("index.md");
    }
}

// ---------------------------------------------------------------------------
//  DisplayArchivePage
// ---------------------------------------------------------------------------

void DocViewPanel::DisplayArchivePage(const std::string& archive_name)
{
    if (!m_is_history_nav)
    {
        RecordNavigation(archive_name);
    }

    if (!wxueArchive.is_open())
    {
        return;
    }

    if (!wxueArchive.DisplayArchivePage(archive_name, *m_html_win))
    {
        return;
    }

    // Reset find-in-page state for the new page
    m_find_last_query.clear();
    m_find_last_pos = 0;
    m_find_highlight_active = false;
    m_find_highlight_occurrence = 0;
    m_find_highlight_query.clear();

    // Inject inheritance graph after </h1> if data is available for this page.
    const wxFileName fn(wxString::FromUTF8(archive_name));
    const std::string class_name = fn.GetName().utf8_string();
    const std::string img_block = BuildInheritanceImage(class_name);
    if (!img_block.empty())
    {
        std::string modified_html = wxueArchive.GetCurrentHtml();
        const std::size_t h1_end_pos = modified_html.find("</h1>");
        constexpr std::size_t H1_CLOSE_LEN = 5;  // length of "</h1>"
        if (h1_end_pos != std::string::npos)
        {
            modified_html.insert(h1_end_pos + H1_CLOSE_LEN, "\n" + img_block);
        }
        else
        {
            modified_html.insert(0, img_block);
        }
        m_html_win->SetPage(wxString::FromUTF8(modified_html));
    }

    SetStatusMessage(wxString::FromUTF8(archive_name));
}

// ---------------------------------------------------------------------------
//  Event handlers — HTML window
// ---------------------------------------------------------------------------

void DocViewPanel::OnHtmlLink(wxHtmlLinkEvent& event)
{
    wxueArchive.OnHtmlLink(event.GetLinkInfo(), *m_html_win);

    const std::string& current_page = wxueArchive.GetCurrentPage();
    if (!current_page.empty())
    {
        RecordNavigation(current_page);
        SetStatusMessage(wxString::FromUTF8(current_page));
    }
}

// ---------------------------------------------------------------------------
//  Event handlers — navigation
// ---------------------------------------------------------------------------

void DocViewPanel::OnHome([[maybe_unused]] wxCommandEvent& event)
{
    NavigateHome();
}

// ---------------------------------------------------------------------------
//  Event handlers — index tabs (classes / events / functions / overviews)
// ---------------------------------------------------------------------------

void DocViewPanel::OnPageChanged(wxBookCtrlEvent& event)
{
    const int page_sel = event.GetSelection();
    if (page_sel == wxNOT_FOUND)
    {
        event.Skip();
        return;
    }

    const wxWindow* const changed_page = m_choicebook->GetPage(page_sel);

    std::string index_file;
    wxListBox* listbox = nullptr;
    wxTextCtrl* textctrl = nullptr;
    wxString default_filter;
    std::vector<std::string>* item_store = nullptr;

    if (changed_page == m_classes_page)
    {
        index_file = "data/classes.md";
        listbox = m_classes_listbox;
        textctrl = m_classes_textctrl;
        default_filter = wxT("wx");
        item_store = &m_classes_items;
    }
    else if (changed_page == m_events_page)
    {
        index_file = "data/events.md";
        listbox = m_events_listbox;
        textctrl = m_events_textctrl;
        default_filter = wxT("wx");
        item_store = &m_events_items;
    }
    else if (changed_page == m_functions_page)
    {
        index_file = "data/functions.md";
        listbox = m_functions_listbox;
        textctrl = m_functions_textctrl;
        item_store = &m_functions_items;
    }
    else if (changed_page == m_overviews_page)
    {
        index_file = "data/overviews.md";
        listbox = m_overviews_listbox;
        textctrl = m_overviews_textctrl;
        item_store = &m_overviews_items;
    }
    else
    {
        // Search page or unknown — nothing to lazy-load
        event.Skip();
        return;
    }

    if (wxueArchive.is_open() && listbox->IsEmpty())
    {
        PopulateIndexListbox(index_file, listbox, textctrl, default_filter, *item_store);
    }
    event.Skip();
}

void DocViewPanel::OnDblClickListBox(wxCommandEvent& event)
{
    const wxString selection = event.GetString();
    if (selection.empty())
    {
        return;
    }

    const std::string archive_name = selection.utf8_string() + ".md";
    DisplayArchivePage(archive_name);
}

void DocViewPanel::OnIndexTextChange(wxCommandEvent& event)
{
    wxListBox* const listbox = GetActiveIndexListbox(event.GetEventObject());
    if (listbox == nullptr)
    {
        event.Skip();
        return;
    }

    const std::vector<std::string>* item_vec = nullptr;
    if (listbox == m_classes_listbox)
    {
        item_vec = &m_classes_items;
    }
    else if (listbox == m_events_listbox)
    {
        item_vec = &m_events_items;
    }
    else if (listbox == m_functions_listbox)
    {
        item_vec = &m_functions_items;
    }
    else if (listbox == m_overviews_listbox)
    {
        item_vec = &m_overviews_items;
    }

    if (item_vec == nullptr)
    {
        event.Skip();
        return;
    }

    ApplyFilter(listbox, *item_vec, event.GetString());
}

void DocViewPanel::OnIndexTextEnter(wxCommandEvent& event)
{
    const wxListBox* const listbox = GetActiveIndexListbox(event.GetEventObject());
    if (listbox == nullptr)
    {
        event.Skip();
        return;
    }

    if (listbox->GetCount() == 1)
    {
        const std::string archive_name = listbox->GetString(0).utf8_string() + ".md";
        DisplayArchivePage(archive_name);
    }
}

void DocViewPanel::OnTextKeyDown(wxKeyEvent& event)
{
    const int key_code = event.GetKeyCode();

    // Find which listbox is on the active choicebook page
    const int page_sel = m_choicebook->GetSelection();
    if (page_sel == wxNOT_FOUND)
    {
        event.Skip();
        return;
    }

    const wxWindow* const changed_page = m_choicebook->GetPage(page_sel);
    wxListBox* listbox = nullptr;

    if (changed_page == m_classes_page)
    {
        listbox = m_classes_listbox;
    }
    else if (changed_page == m_events_page)
    {
        listbox = m_events_listbox;
    }
    else if (changed_page == m_functions_page)
    {
        listbox = m_functions_listbox;
    }
    else if (changed_page == m_overviews_page)
    {
        listbox = m_overviews_listbox;
    }
    else if (changed_page == m_search_page)
    {
        listbox = m_search_listbox;
    }

    if (listbox == nullptr || listbox->GetCount() == 0)
    {
        event.Skip();
        return;
    }

    // Down arrow: move selection down, wrapping at bottom
    if (key_code == WXK_DOWN)
    {
        const int selection = listbox->GetSelection();
        if (selection != wxNOT_FOUND && selection + 1 < static_cast<int>(listbox->GetCount()))
        {
            listbox->SetSelection(selection + 1);
        }
        else
        {
            listbox->SetSelection(0);
        }
        return;
    }

    // Up arrow: move selection up, wrapping at top
    if (key_code == WXK_UP)
    {
        const int selection = listbox->GetSelection();
        if (selection != wxNOT_FOUND && selection > 0)
        {
            listbox->SetSelection(selection - 1);
        }
        else
        {
            listbox->SetSelection(static_cast<int>(listbox->GetCount()) - 1);
        }
        return;
    }

    // Enter: display the selected item
    if (key_code == WXK_RETURN)
    {
        const int selection = listbox->GetSelection();
        if (selection != wxNOT_FOUND)
        {
            const std::string archive_name = listbox->GetString(selection).utf8_string() + ".md";
            DisplayArchivePage(archive_name);
            return;
        }
    }

    event.Skip();
}

// ---------------------------------------------------------------------------
//  Event handlers — search
// ---------------------------------------------------------------------------

void DocViewPanel::OnSearchTextChanged(wxCommandEvent& event)
{
    const wxString query = event.GetString();
    m_search_listbox->Clear();

    if (query.empty() || !wxueArchive.HasSearchIndex())
    {
        return;
    }

    const std::string query_utf8 = query.utf8_string();
    std::expected<std::vector<ftsrch::QueryResult>, ftsrch::Error> results =
        wxueArchive.SearchIncremental(query_utf8);
    if (!results)
    {
        return;
    }

    for (const ftsrch::QueryResult& result: *results)
    {
        const std::string archive_path = wxueArchive.GetArchivePathForDoc(result.doc_id);
        if (!archive_path.empty())
        {
            // Strip the .md extension for display
            const wxFileName fn(wxString::FromUTF8(archive_path));
            const std::string display_name = fn.GetName().utf8_string();
            m_search_listbox->Append(wxString::FromUTF8(display_name));
        }
    }
}

void DocViewPanel::OnSearchCancel([[maybe_unused]] wxCommandEvent& event)
{
    m_search_ctrl->Clear();
    m_search_listbox->Clear();
}

void DocViewPanel::OnDisplaySearchListItem(wxCommandEvent& event)
{
    const wxString selection = event.GetString();
    if (selection.empty())
    {
        return;
    }

    const std::string archive_name = selection.utf8_string() + ".md";
    DisplayArchivePage(archive_name);
}

// ---------------------------------------------------------------------------
//  Find-in-page
// ---------------------------------------------------------------------------

void DocViewPanel::OnFind([[maybe_unused]] wxCommandEvent& event)
{
    if (!wxueArchive.is_open())
    {
        SetStatusMessage("No archive loaded");
        return;
    }

    const std::string& markdown = wxueArchive.GetCurrentMarkdown();
    if (markdown.empty())
    {
        SetStatusMessage("No page loaded");
        return;
    }

    // Create a simple modal find dialog
    wxDialog find_dlg(this, wxID_ANY, wxT("Find in page"), wxDefaultPosition);
    wxBoxSizer* const find_sizer = new wxBoxSizer(wxVERTICAL);

    wxTextCtrl* const text_ctrl = new wxTextCtrl(
        &find_dlg, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_PROCESS_ENTER);
    find_sizer->Add(text_ctrl, wxSizerFlags().Expand().Border(wxALL, 8));

    wxBoxSizer* const btn_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* const find_next_btn = new wxButton(&find_dlg, wxID_FORWARD, wxT("&Find Next"));
    wxButton* const cancel_btn = new wxButton(&find_dlg, wxID_CANCEL, wxT("&Close"));
    btn_sizer->Add(find_next_btn, wxSizerFlags().Border(wxRIGHT, 8));
    btn_sizer->Add(cancel_btn, wxSizerFlags());
    find_sizer->Add(btn_sizer, wxSizerFlags().Center().Border(wxBOTTOM, 8));

    find_dlg.SetSizerAndFit(find_sizer);

    wxString search_text;
    bool search_requested = false;

    text_ctrl->Bind(wxEVT_TEXT_ENTER,
                    [&]([[maybe_unused]] wxCommandEvent& event_arg)
                    {
                        search_text = text_ctrl->GetValue();
                        search_requested = true;
                        find_dlg.EndModal(wxID_OK);
                    });

    find_next_btn->Bind(wxEVT_BUTTON,
                        [&]([[maybe_unused]] wxCommandEvent& event_arg)
                        {
                            search_text = text_ctrl->GetValue();
                            search_requested = true;
                            find_dlg.EndModal(wxID_OK);
                        });

    find_dlg.ShowModal();

    if (!search_requested || search_text.empty())
    {
        return;
    }

    const std::string query = search_text.utf8_string();

    // Reset position when the query changes
    if (query != m_find_last_query)
    {
        m_find_last_query = query;
        m_find_last_pos = 0;
    }

    // Search the raw markdown for the query, starting from the last position
    std::size_t found_pos = FindInMarkdown(markdown, query, m_find_last_pos);
    bool wrapped = false;
    if (found_pos == std::string::npos && m_find_last_pos > 0)
    {
        // Wrap around: retry from the beginning
        found_pos = FindInMarkdown(markdown, query, 0);
        if (found_pos != std::string::npos)
        {
            wrapped = true;
        }
    }
    if (found_pos == std::string::npos)
    {
        SetStatusMessage(wxString::Format("Not found: %s", search_text));
        return;
    }

    m_find_last_pos = found_pos + query.size();

    // Count which occurrence this is (0-based)
    const int occurrence_index = CountOccurrencesBefore(markdown, query, found_pos);

    // Get the clean HTML and apply highlight
    const std::string clean_html = wxueArchive.GetCurrentHtml();
    const std::string highlighted_html = ApplyFindHighlight(clean_html, query, occurrence_index);

    if (highlighted_html == clean_html)
    {
        // Highlight application failed (e.g., occurrence not found in HTML)
        SetStatusMessage(
            wxString::Format("Found \"%s\" but could not highlight in HTML", search_text));
        return;
    }

    // Set the highlighted page
    const bool page_set = m_html_win->SetPage(wxString::FromUTF8(highlighted_html));
    if (!page_set)
    {
        SetStatusMessage(
            wxString::Format("Found \"%s\" but failed to set highlighted page", search_text));
        return;
    }

    // Scroll to the match anchor
    const bool scrolled = m_html_win->LoadPage(wxT("#find-match"));
    if (!scrolled)
    {
        SetStatusMessage(
            wxString::Format("Found \"%s\" but failed to scroll to match", search_text));
        return;
    }

    // Center the match vertically: scroll up by ~1/3 of client height
    {
        int view_x = 0, view_y = 0;
        m_html_win->GetViewStart(&view_x, &view_y);
        int px_per_unit_x = 1, px_per_unit_y = 1;
        m_html_win->GetScrollPixelsPerUnit(&px_per_unit_x, &px_per_unit_y);
        if (px_per_unit_y < 1)
        {
            px_per_unit_y = 1;
        }
        const int client_height = m_html_win->GetClientSize().GetHeight();
        const int scroll_up_pixels = client_height / 3;
        const int scroll_up_units = scroll_up_pixels / px_per_unit_y;
        const int new_scroll_y = std::max(0, view_y - scroll_up_units);
        m_html_win->Scroll(view_x, new_scroll_y);
    }

    // Update highlight tracking state
    m_find_highlight_active = true;
    m_find_highlight_occurrence = occurrence_index;
    m_find_highlight_query = query;

    // Status message
    if (wrapped)
    {
        SetStatusMessage(wxString::Format("Wrapped; found \"%s\"", search_text));
    }
    else
    {
        SetStatusMessage(wxString::Format("Found \"%s\"", search_text));
    }
}

void DocViewPanel::OnFindNext(wxKeyEvent& event)
{
    // 1. Guard: if no active find highlight, skip the event and return
    if (!m_find_highlight_active || m_find_highlight_query.empty())
    {
        event.Skip();
        return;
    }

    // 2. Get current markdown
    const std::string& markdown = wxueArchive.GetCurrentMarkdown();
    if (markdown.empty())
    {
        event.Skip();
        return;
    }

    const std::string& query = m_find_highlight_query;

    // 3. Search markdown starting from m_find_last_pos (right after current match)
    std::size_t found_pos = FindInMarkdown(markdown, query, m_find_last_pos);
    bool wrapped = false;

    // 4. If not found, wrap around: search from beginning
    if (found_pos == std::string::npos)
    {
        found_pos = FindInMarkdown(markdown, query, 0);
        if (found_pos != std::string::npos)
        {
            // Check if this is the SAME match we're already on (only one occurrence)
            // Current match starts at m_find_last_pos - query.size()
            const std::size_t current_match_start = m_find_last_pos - query.size();
            if (found_pos == current_match_start)
            {
                SetStatusMessage(wxString::Format("Only one occurrence of \"%s\" found",
                                                  wxString::FromUTF8(query)));
                return;
            }
            wrapped = true;
        }
    }

    // 5. If still not found (shouldn't happen if we have an active highlight, but be safe)
    if (found_pos == std::string::npos)
    {
        SetStatusMessage(wxString::Format("Not found: \"%s\"", wxString::FromUTF8(query)));
        return;
    }

    // 6. Update position
    m_find_last_pos = found_pos + query.size();

    // 7. Count which occurrence this is (0-based)
    const int occurrence_index = CountOccurrencesBefore(markdown, query, found_pos);

    // 8. Get clean HTML and apply highlight (ApplyFindHighlight internally removes old highlight)
    const std::string clean_html = wxueArchive.GetCurrentHtml();
    const std::string highlighted_html = ApplyFindHighlight(clean_html, query, occurrence_index);

    if (highlighted_html == clean_html)
    {
        SetStatusMessage(wxString::Format("Found \"%s\" but could not highlight in HTML",
                                          wxString::FromUTF8(query)));
        return;
    }

    // 9. Set the highlighted page
    const bool page_set = m_html_win->SetPage(wxString::FromUTF8(highlighted_html));
    if (!page_set)
    {
        SetStatusMessage(wxString::Format("Found \"%s\" but failed to set highlighted page",
                                          wxString::FromUTF8(query)));
        return;
    }

    // 10. Scroll to the match anchor
    const bool scrolled = m_html_win->LoadPage(wxT("#find-match"));
    if (!scrolled)
    {
        SetStatusMessage(wxString::Format("Found \"%s\" but failed to scroll to match",
                                          wxString::FromUTF8(query)));
        return;
    }

    // 11. Center the match vertically (same offset logic as OnFind)
    {
        int view_x = 0, view_y = 0;
        m_html_win->GetViewStart(&view_x, &view_y);
        int px_per_unit_x = 1, px_per_unit_y = 1;
        m_html_win->GetScrollPixelsPerUnit(&px_per_unit_x, &px_per_unit_y);
        if (px_per_unit_y < 1)
        {
            px_per_unit_y = 1;
        }
        const int client_height = m_html_win->GetClientSize().GetHeight();
        const int scroll_up_pixels = client_height / 3;
        const int scroll_up_units = scroll_up_pixels / px_per_unit_y;
        const int new_scroll_y = std::max(0, view_y - scroll_up_units);
        m_html_win->Scroll(view_x, new_scroll_y);
    }

    // 12. Update highlight tracking state
    m_find_highlight_occurrence = occurrence_index;

    // 13. Status message
    if (wrapped)
    {
        SetStatusMessage(wxString::Format("Wrapped; found \"%s\"", wxString::FromUTF8(query)));
    }
    else
    {
        SetStatusMessage(wxString::Format("Found \"%s\"", wxString::FromUTF8(query)));
    }
}

// ---------------------------------------------------------------------------
//  Private helpers
// ---------------------------------------------------------------------------

std::string DocViewPanel::RemoveFindHighlight(const std::string& html)
{
    constexpr std::string_view ANCHOR_PREFIX =
        "<a id=\"find-match\"></a><span style=\"background-color: #FFFF00; color: #000000;\">";
    const std::size_t start_pos = html.find(ANCHOR_PREFIX);
    if (start_pos == std::string::npos)
    {
        return html;
    }
    const std::size_t content_start = start_pos + ANCHOR_PREFIX.size();
    const std::size_t span_close_pos = html.find("</span>", content_start);
    if (span_close_pos == std::string::npos)
    {
        return html;
    }
    const std::string inner_text = html.substr(content_start, span_close_pos - content_start);
    constexpr std::size_t CLOSE_TAG_LEN = 7;  // length of "</span>"
    std::string cleaned = html;
    cleaned.erase(start_pos, span_close_pos + CLOSE_TAG_LEN - start_pos);
    cleaned.insert(start_pos, inner_text);
    return cleaned;
}

int DocViewPanel::CountOccurrencesBefore(std::string_view text, std::string_view query,
                                         std::size_t pos)
{
    if (query.empty())
    {
        return 0;
    }
    int count = 0;
    const std::size_t limit = std::min(pos, text.size());
    for (std::size_t idx = 0; idx + query.size() <= limit; ++idx)
    {
        if (std::tolower(static_cast<unsigned char>(text[idx])) ==
            std::tolower(static_cast<unsigned char>(query[0])))
        {
            bool match = true;
            for (std::size_t jdx = 1; jdx < query.size(); ++jdx)
            {
                if (std::tolower(static_cast<unsigned char>(text[idx + jdx])) !=
                    std::tolower(static_cast<unsigned char>(query[jdx])))
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                ++count;
            }
        }
    }
    return count;
}

std::string DocViewPanel::ApplyFindHighlight(const std::string& html, const std::string& query,
                                             int occurrence_index)
{
    const std::string cleaned = RemoveFindHighlight(html);
    if (query.empty() || occurrence_index < 0)
    {
        return cleaned;
    }

    constexpr std::string_view ANCHOR_PREFIX = "<a id=\"find-match\"></a>";
    constexpr std::string_view SPAN_PREFIX =
        "<span style=\"background-color: #FFFF00; color: #000000;\">";

    const std::size_t query_len = query.size();
    int match_index = 0;
    std::size_t idx = 0;
    const std::size_t len = cleaned.size();

    while (idx < len)
    {
        if (cleaned[idx] == '<')
        {
            const std::size_t gt_pos = cleaned.find('>', idx);
            if (gt_pos == std::string::npos)
            {
                break;
            }
            idx = gt_pos + 1;
            continue;
        }
        if (idx + query_len > len)
        {
            break;
        }
        if (std::tolower(static_cast<unsigned char>(cleaned[idx])) ==
            std::tolower(static_cast<unsigned char>(query[0])))
        {
            bool match = true;
            for (std::size_t jdx = 1; jdx < query_len; ++jdx)
            {
                if (std::tolower(static_cast<unsigned char>(cleaned[idx + jdx])) !=
                    std::tolower(static_cast<unsigned char>(query[jdx])))
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                if (match_index == occurrence_index)
                {
                    const std::string matched_text = cleaned.substr(idx, query_len);
                    std::string result = cleaned;
                    result.erase(idx, query_len);
                    std::string replacement;
                    replacement.reserve(ANCHOR_PREFIX.size() + SPAN_PREFIX.size() + query_len + 7);
                    replacement += ANCHOR_PREFIX;
                    replacement += SPAN_PREFIX;
                    replacement += matched_text;
                    replacement += "</span>";
                    result.insert(idx, replacement);
                    return result;
                }
                ++match_index;
                idx += query_len;
                continue;
            }
        }
        ++idx;
    }

    return cleaned;
}

std::string DocViewPanel::BuildInheritanceImage(const std::string& class_name)
{
    const std::unordered_map<std::string, InheritEntry>::const_iterator found =
        m_inherit_map.find(class_name);
    if (found == m_inherit_map.end())
    {
        return {};
    }
    const InheritEntry& entry = found->second;
    if (entry.bases.empty() && entry.derived.empty())
    {
        return {};
    }

    constexpr std::size_t MAX_CHILDREN = 24;
    std::vector<docparser::InheritGraphNode> nodes;
    nodes.reserve(entry.bases.size() + entry.derived.size() + 2);

    for (const std::string& base_class: entry.bases)
    {
        docparser::InheritGraphNode base_node;
        base_node.name = base_class;
        base_node.url = base_class + ".md";
        nodes.push_back(std::move(base_node));
    }

    {
        docparser::InheritGraphNode self_node;
        self_node.name = class_name;
        self_node.highlight = true;
        self_node.bases = entry.bases;
        nodes.push_back(std::move(self_node));
    }

    const std::size_t shown = std::min(entry.derived.size(), MAX_CHILDREN);
    for (std::size_t idx = 0; idx < shown; ++idx)
    {
        docparser::InheritGraphNode child_node;
        child_node.name = entry.derived[idx];
        child_node.url = entry.derived[idx] + ".md";
        child_node.bases = { class_name };
        nodes.push_back(std::move(child_node));
    }

    if (entry.derived.size() > shown)
    {
        docparser::InheritGraphNode more_node;
        more_node.name = "(+" + std::to_string(entry.derived.size() - shown) + " more)";
        more_node.bases = { class_name };
        nodes.push_back(std::move(more_node));
    }

    const bool use_horizontal = (entry.bases.size() <= 1 && entry.derived.size() > 8);

    double svg_width = 0.0;
    double svg_height = 0.0;
    const std::string svg =
        docparser::RenderInheritanceSvg(nodes, &svg_width, &svg_height, use_horizontal);
    if (svg.empty() || svg_width < 1.0 || svg_height < 1.0)
    {
        return {};
    }

    // Remove the previous graph image to avoid accumulating stale memory-FS files.
    if (!m_current_graph_fs_name.empty())
    {
        wxMemoryFSHandler::RemoveFile(wxString::FromUTF8(m_current_graph_fs_name));
    }
    ++m_graph_counter;
    m_current_graph_fs_name = "inherit_" + std::to_string(m_graph_counter) + ".svg";
    wxMemoryFSHandler::AddFileWithMimeType(wxString::FromUTF8(m_current_graph_fs_name), svg.data(),
                                           svg.size(), "image/svg+xml");

    return "<p><img src=\"memory:" + m_current_graph_fs_name + "\" alt=\"Inheritance graph for " +
           class_name + "\"></p>\n";
}

void DocViewPanel::PopulateIndexListbox(const std::string& index_file, wxListBox* listbox,
                                        wxTextCtrl* filter_ctrl, const wxString& default_filter,
                                        std::vector<std::string>& item_store)
{
    const std::expected<std::string, std::string> result = wxueArchive.ReadFile(index_file);
    if (!result)
    {
        return;
    }

    item_store.clear();
    std::istringstream stream(*result);
    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (!line.empty())
        {
            item_store.push_back(line);
        }
    }

    filter_ctrl->SetValue(default_filter);
    ApplyFilter(listbox, item_store, default_filter);
}

void DocViewPanel::ApplyFilter(wxListBox* listbox, const std::vector<std::string>& all_items,
                               const wxString& filter_text)
{
    const std::string filter = filter_text.utf8_string();

    listbox->Freeze();
    listbox->Clear();
    for (const std::string& item: all_items)
    {
        if (filter.empty() || item.contains(filter))
        {
            listbox->Append(wxString(item));
        }
    }
    listbox->Thaw();
}

wxListBox* DocViewPanel::GetActiveIndexListbox(const wxObject* source) const
{
    if (source == m_classes_textctrl)
    {
        return m_classes_listbox;
    }
    if (source == m_events_textctrl)
    {
        return m_events_listbox;
    }
    if (source == m_functions_textctrl)
    {
        return m_functions_listbox;
    }
    if (source == m_overviews_textctrl)
    {
        return m_overviews_listbox;
    }
    return nullptr;
}

void DocViewPanel::SetStatusMessage(const wxString& msg)
{
    wxFrame* const frame = wxDynamicCast(wxGetTopLevelParent(this), wxFrame);
    if (frame != nullptr)
    {
        frame->SetStatusText(msg);
    }
}

void DocViewPanel::RecordNavigation(const std::string& destination)
{
    if (!m_current_history_page.empty())
    {
        m_back_history.push_back(m_current_history_page);
    }
    m_forward_history.clear();
    m_current_history_page = destination;
}

void DocViewPanel::OnNavBack([[maybe_unused]] wxCommandEvent& event)
{
    if (m_back_history.empty())
    {
        return;
    }

    m_forward_history.push_back(m_current_history_page);

    const std::string target = m_back_history.back();
    m_back_history.pop_back();

    m_current_history_page = target;

    m_is_history_nav = true;
    DisplayArchivePage(target);
    m_is_history_nav = false;
}

void DocViewPanel::OnNavForward([[maybe_unused]] wxCommandEvent& event)
{
    if (m_forward_history.empty())
    {
        return;
    }

    m_back_history.push_back(m_current_history_page);

    const std::string target = m_forward_history.back();
    m_forward_history.pop_back();

    m_current_history_page = target;

    m_is_history_nav = true;
    DisplayArchivePage(target);
    m_is_history_nav = false;
}

void DocViewPanel::OnUpdateUI([[maybe_unused]] wxUpdateUIEvent& event)
{
    m_aui_tool_bar->EnableTool(m_back->GetId(), !m_back_history.empty());
    m_aui_tool_bar->EnableTool(m_forward->GetId(), !m_forward_history.empty());
}
