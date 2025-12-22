/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for displaying file differences
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/dialog.h>
#include <wx/stc/stc.h>

#include <string>
#include <vector>

#include "diff.h"  // DiffResult, FileDiff

class wxBoxSizer;
class wxButton;
class wxChoice;
class wxStaticText;

class DiffViewer : public wxDialog
{
public:
    DiffViewer(wxWindow* parent, const std::vector<FileDiff>& diffs);

private:
    void CreateControls();
    void DisplayDiff(size_t index);
    void SetupTextControl(wxStyledTextCtrl* ctrl);
    void ApplySyntaxHighlighting(wxStyledTextCtrl* ctrl, const std::string& filename);

    // Event handlers
    void OnFileChanged(wxCommandEvent& event);
    void OnPrevious(wxCommandEvent& event);
    void OnNext(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);

    wxChoice* m_file_choice;
    wxStyledTextCtrl* m_original_text;
    wxStyledTextCtrl* m_modified_text;
    wxButton* m_prev_button;
    wxButton* m_next_button;
    wxStaticText* m_diff_info;

    std::vector<FileDiff> m_diffs;
    size_t m_current_index { 0 };
};
