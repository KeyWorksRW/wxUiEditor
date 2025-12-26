/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for displaying file differences
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "diff_viewer.h"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "wxue_namespace/wxue_string.h"  // wxue::string

DiffViewer::DiffViewer(wxWindow* parent, const std::vector<FileDiff>& diffs) :
    wxDialog(parent, wxID_ANY, "Code Differences", wxDefaultPosition, wxSize(1200, 800),
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX),
    m_diffs(diffs)
{
    CreateControls();

    if (!m_diffs.empty())
    {
        DisplayDiff(0);
    }
}

void DiffViewer::CreateControls()
{
    auto* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Top panel with file selection
    auto* top_sizer = new wxBoxSizer(wxHORIZONTAL);

    auto* file_label = new wxStaticText(this, wxID_ANY, "File:");
    top_sizer->Add(file_label, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_file_choice = new wxChoice(this, wxID_ANY);
    for (const auto& diff: m_diffs)
    {
        m_file_choice->Append(diff.filename);
    }
    if (!m_diffs.empty())
    {
        m_file_choice->SetSelection(0);
    }
    m_file_choice->Bind(wxEVT_CHOICE, &DiffViewer::OnFileChanged, this);
    top_sizer->Add(m_file_choice, 1, wxEXPAND | wxALL, 5);

    m_prev_button = new wxButton(this, wxID_ANY, "< Previous");
    m_prev_button->Bind(wxEVT_BUTTON, &DiffViewer::OnPrevious, this);
    top_sizer->Add(m_prev_button, 0, wxALL, 5);

    m_next_button = new wxButton(this, wxID_ANY, "Next >");
    m_next_button->Bind(wxEVT_BUTTON, &DiffViewer::OnNext, this);
    top_sizer->Add(m_next_button, 0, wxALL, 5);

    main_sizer->Add(top_sizer, 0, wxEXPAND | wxALL, 5);

    // Info text
    m_diff_info = new wxStaticText(this, wxID_ANY, "");
    main_sizer->Add(m_diff_info, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Labels for the two panes
    auto* label_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto* orig_label = new wxStaticText(this, wxID_ANY, "Original (On Disk)");
    label_sizer->Add(orig_label, 1, wxALL, 5);
    auto* new_label = new wxStaticText(this, wxID_ANY, "Generated (New)");
    label_sizer->Add(new_label, 1, wxALL, 5);
    main_sizer->Add(label_sizer, 0, wxEXPAND);

    // Split view with two text controls
    auto* text_sizer = new wxBoxSizer(wxHORIZONTAL);

    m_original_text = new wxStyledTextCtrl(this, wxID_ANY);
    SetupTextControl(m_original_text);
    text_sizer->Add(m_original_text, 1, wxEXPAND | wxALL, 5);

    m_modified_text = new wxStyledTextCtrl(this, wxID_ANY);
    SetupTextControl(m_modified_text);
    text_sizer->Add(m_modified_text, 1, wxEXPAND | wxALL, 5);

    main_sizer->Add(text_sizer, 1, wxEXPAND);

    // Bottom buttons
    auto* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    button_sizer->AddStretchSpacer();

    auto* close_button = new wxButton(this, wxID_OK, "Close");
    close_button->Bind(wxEVT_BUTTON, &DiffViewer::OnClose, this);
    button_sizer->Add(close_button, 0, wxALL, 5);

    main_sizer->Add(button_sizer, 0, wxEXPAND | wxALL, 5);

    SetSizer(main_sizer);
}

void DiffViewer::SetupTextControl(wxStyledTextCtrl* ctrl)
{
    ctrl->SetReadOnly(false);  // Temporarily allow modifications for setup

    // Basic setup
    ctrl->StyleClearAll();
    ctrl->SetLexer(wxSTC_LEX_CPP);

    // Line numbers
    ctrl->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    ctrl->SetMarginWidth(0, 50);

    // Colors for diff highlighting
    // Style 0: Normal text (unchanged)
    ctrl->StyleSetForeground(0, wxColour(0, 0, 0));
    ctrl->StyleSetBackground(0, wxColour(255, 255, 255));

    // Style 1: Deleted lines (red background)
    ctrl->StyleSetForeground(1, wxColour(0, 0, 0));
    ctrl->StyleSetBackground(1, wxColour(255, 200, 200));

    // Style 2: Added lines (green background)
    ctrl->StyleSetForeground(2, wxColour(0, 0, 0));
    ctrl->StyleSetBackground(2, wxColour(200, 255, 200));

    // Style 3: Modified lines (yellow background)
    ctrl->StyleSetForeground(3, wxColour(0, 0, 0));
    ctrl->StyleSetBackground(3, wxColour(255, 255, 200));

    // Font
    wxFont font(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    ctrl->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

    ctrl->SetReadOnly(true);
}

void DiffViewer::ApplySyntaxHighlighting(wxStyledTextCtrl* ctrl, const std::string& filename)
{
    // Determine language based on file extension
    if (filename.ends_with(".cpp") || filename.ends_with(".cc") || filename.ends_with(".cxx") ||
        filename.ends_with(".h") || filename.ends_with(".hpp"))
    {
        ctrl->SetLexer(wxSTC_LEX_CPP);
    }
    else if (filename.ends_with(".py"))
    {
        ctrl->SetLexer(wxSTC_LEX_PYTHON);
    }
    else if (filename.ends_with(".rb"))
    {
        ctrl->SetLexer(wxSTC_LEX_RUBY);
    }
    else if (filename.ends_with(".pl") || filename.ends_with(".pm"))
    {
        ctrl->SetLexer(wxSTC_LEX_PERL);
    }
    else
    {
        ctrl->SetLexer(wxSTC_LEX_NULL);
    }
}

void DiffViewer::DisplayDiff(size_t index)
{
    if (index >= m_diffs.size())
    {
        return;
    }

    m_current_index = index;
    const auto& diff = m_diffs[index];

    // Update file selection
    m_file_choice->SetSelection(static_cast<int>(index));

    // Update navigation buttons
    m_prev_button->Enable(index > 0);
    m_next_button->Enable(index < m_diffs.size() - 1);

    // Update info text
    if (diff.diff_result.has_differences)
    {
        size_t added = 0;
        size_t deleted = 0;
        for (const auto& line: diff.diff_result.left_lines)
        {
            if (line.type == DiffType::deleted)
            {
                ++deleted;
            }
        }
        for (const auto& line: diff.diff_result.right_lines)
        {
            if (line.type == DiffType::added)
            {
                ++added;
            }
        }

        wxue::string info;
        info << "File " << (index + 1) << " of " << m_diffs.size() << " - " << added
             << " lines added, " << deleted << " lines deleted";
        m_diff_info->SetLabel(info.wx());
    }
    else
    {
        wxue::string info;
        info << "File " << (index + 1) << " of " << m_diffs.size() << " - No differences";
        m_diff_info->SetLabel(info.wx());
    }

    // Clear and populate text controls
    m_original_text->SetReadOnly(false);
    m_modified_text->SetReadOnly(false);

    m_original_text->ClearAll();
    m_modified_text->ClearAll();

    // Build the display text with line-by-line coloring
    for (size_t line_idx = 0; line_idx < diff.diff_result.left_lines.size(); ++line_idx)
    {
        const auto& left_line = diff.diff_result.left_lines[line_idx];
        const auto& right_line = diff.diff_result.right_lines[line_idx];

        // Add text to left pane
        int left_pos = m_original_text->GetLength();
        m_original_text->AppendText(left_line.text + "\n");
        int left_end = m_original_text->GetLength();

        // Add text to right pane
        int right_pos = m_modified_text->GetLength();
        m_modified_text->AppendText(right_line.text + "\n");
        int right_end = m_modified_text->GetLength();

        // Apply styling based on diff type
        int style = 0;
        switch (left_line.type)
        {
            case DiffType::unchanged:
                style = 0;
                break;
            case DiffType::deleted:
                style = 1;
                break;
            case DiffType::added:
                style = 2;
                break;
            case DiffType::modified:
                style = 3;
                break;
        }

        if (left_line.type != DiffType::unchanged)
        {
            m_original_text->StartStyling(left_pos);
            m_original_text->SetStyling(left_end - left_pos, style);
        }

        if (right_line.type != DiffType::unchanged)
        {
            m_modified_text->StartStyling(right_pos);
            m_modified_text->SetStyling(right_end - right_pos, style);
        }
    }

    m_original_text->SetReadOnly(true);
    m_modified_text->SetReadOnly(true);
}

void DiffViewer::OnFileChanged(wxCommandEvent& event)
{
    DisplayDiff(event.GetSelection());
}

void DiffViewer::OnPrevious(wxCommandEvent& /* event */)
{
    if (m_current_index > 0)
    {
        DisplayDiff(m_current_index - 1);
    }
}

void DiffViewer::OnNext(wxCommandEvent& /* event */)
{
    if (m_current_index < m_diffs.size() - 1)
    {
        DisplayDiff(m_current_index + 1);
    }
}

void DiffViewer::OnClose(wxCommandEvent& /* event */)
{
    EndModal(wxID_OK);
}
