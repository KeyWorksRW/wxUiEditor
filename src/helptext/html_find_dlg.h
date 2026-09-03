/////////////////////////////////////////////////////////////////////////////
// Purpose:   Reusable Find dialog for wxHtmlWindow
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

#include "generated/html_find_dlg_base.h"

class wxHtmlWindow;

class HtmlFindDlg : public HtmlFindDlgBase
{
public:
    HtmlFindDlg();  // If you use this constructor, you must call Create(parent)
    HtmlFindDlg(wxWindow* parent);

    // Set the wxHtmlWindow that this dialog searches in. Must be called after
    // Create(parent) and before Show().
    void SetHtmlWindow(wxHtmlWindow* html_win);

    // Re-run the current query (used by the panel when F3 is pressed).
    // Returns true if a match was found and displayed.
    bool FindNextFromPanel();

    // Reset all find state (called when the panel navigates to a new page).
    void ResetForNewPage();

protected:
    // Handlers for HtmlFindDlgBase events
    void OnInit(wxInitDialogEvent& event) override;
    void OnFindTextEnter(wxCommandEvent& event) override;
    void OnText(wxCommandEvent& event) override;
    void OnFindNext(wxCommandEvent& event) override;

    // Highlight a match at the given markdown position. Returns false if the
    // match could not be highlighted or displayed.
    bool ShowMatchAt(const std::string& query, std::size_t found_pos);

    // Search for the next match (used by Find Next button and Enter key).
    bool FindNextMatch();

private:
    wxHtmlWindow* m_html_win { nullptr };
    std::string m_find_last_query;
    std::size_t m_find_last_pos { 0 };
    bool m_find_highlight_active { false };
    int m_find_highlight_occurrence { 0 };
    std::string m_find_highlight_query;
};
