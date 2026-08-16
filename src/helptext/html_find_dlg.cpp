/////////////////////////////////////////////////////////////////////////////
// Purpose:   Reusable Find dialog for wxHtmlWindow
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "html_find_dlg.h"  // auto-generated: ..\..\wxui\html_find_dlg_base.h and ..\..\wxui\html_find_dlg_base.cpp

#include <algorithm>
#include <cctype>

#include <wx/html/htmlwin.h>
#include <wx/textctrl.h>

#include "archive_handler.h"
#include "find_in_page.h"

// If this constructor is used, the caller must call Create(parent)
HtmlFindDlg::HtmlFindDlg() {}

HtmlFindDlg::HtmlFindDlg(wxWindow* parent)
{
    if (!Create(parent))
    {
        return;
    }
}

void HtmlFindDlg::SetHtmlWindow(wxHtmlWindow* html_win)
{
    m_html_win = html_win;
}

void HtmlFindDlg::ResetForNewPage()
{
    m_find_last_query.clear();
    m_find_last_pos = 0;
    m_find_highlight_active = false;
    m_find_highlight_occurrence = 0;
    m_find_highlight_query.clear();
}

void HtmlFindDlg::OnInit(wxInitDialogEvent& event)
{
    event.Skip();  // transfer all validator data to their windows and update UI

    // Start with focus in the find text control and preselect any existing text
    m_findTextCtrl->SetFocus();
    m_findTextCtrl->SelectAll();
}

// Find Next button or Enter in the text control
void HtmlFindDlg::OnFindNext(wxCommandEvent& /* event unused */)
{
    FindNextMatch();
}

// Enter in the text control
void HtmlFindDlg::OnFindTextEnter(wxCommandEvent& /* event unused */)
{
    FindNextMatch();
}

// Text changed: reset the search position so the next find starts from the top
void HtmlFindDlg::OnText(wxCommandEvent& /* event unused */)
{
    const std::string query = m_findTextCtrl->GetValue().utf8_string();

    if (query != m_find_last_query)
    {
        m_find_last_query = query;
        m_find_last_pos = 0;
    }

    // If the dialog is already visible, immediately jump to the first match
    // so the user gets live feedback while typing. The user must have pressed
    // Find Next at least once for a highlight to be active.
    if (IsShown() && !query.empty() && m_find_highlight_active)
    {
        FindNextMatch();
    }
}

bool HtmlFindDlg::FindNextMatch()
{
    if (m_html_win == nullptr)
    {
        return false;
    }

    const std::string query = m_findTextCtrl->GetValue().utf8_string();
    if (query.empty())
    {
        return false;
    }

    const std::string& markdown = wxueArchive.GetCurrentMarkdown();
    if (markdown.empty())
    {
        return false;
    }

    // Reset position when the query changes
    if (query != m_find_last_query)
    {
        m_find_last_query = query;
        m_find_last_pos = 0;
    }

    // Search the raw markdown for the query, starting from the last position
    std::size_t found_pos = FindInMarkdown(markdown, query, m_find_last_pos);
    if (found_pos == std::string::npos && m_find_last_pos > 0)
    {
        // Wrap around: retry from the beginning
        found_pos = FindInMarkdown(markdown, query, 0);
    }
    if (found_pos == std::string::npos)
    {
        return false;
    }

    m_find_last_pos = found_pos + query.size();

    return ShowMatchAt(query, found_pos);
}

bool HtmlFindDlg::FindNextFromPanel()
{
    // Guard: if no active find highlight, nothing to advance to
    if (!m_find_highlight_active || m_find_highlight_query.empty())
    {
        return false;
    }

    const std::string& markdown = wxueArchive.GetCurrentMarkdown();
    if (markdown.empty())
    {
        return false;
    }

    const std::string& query = m_find_highlight_query;

    // Search markdown starting from m_find_last_pos (right after current match)
    std::size_t found_pos = FindInMarkdown(markdown, query, m_find_last_pos);

    // If not found, wrap around: search from beginning
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
                return false;
            }
        }
    }

    // If still not found (shouldn't happen if we have an active highlight, but be safe)
    if (found_pos == std::string::npos)
    {
        return false;
    }

    m_find_last_pos = found_pos + query.size();

    return ShowMatchAt(query, found_pos);
}

bool HtmlFindDlg::ShowMatchAt(const std::string& query, std::size_t found_pos)
{
    // Count which occurrence this is (0-based)
    const int occurrence_index =
        CountOccurrencesBefore(wxueArchive.GetCurrentMarkdown(), query, found_pos);

    // Get the clean HTML and apply highlight (ApplyFindHighlight internally removes old highlight)
    const std::string clean_html = wxueArchive.GetCurrentHtml();
    const std::string highlighted_html = ApplyFindHighlight(clean_html, query, occurrence_index);

    if (highlighted_html == clean_html)
    {
        return false;
    }

    // Set the highlighted page
    const bool page_set = m_html_win->SetPage(wxString::FromUTF8(highlighted_html));
    if (!page_set)
    {
        return false;
    }

    // Scroll to the match anchor
    const bool scrolled = m_html_win->LoadPage(wxT("#find-match"));
    if (!scrolled)
    {
        return false;
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

    return true;
}
