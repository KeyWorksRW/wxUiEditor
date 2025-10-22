/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to display if wxUiEditor is launched with no arguments
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/filename.h>  // wxFileName class

#include "../wxui/startup_dlg_base.h"

class StartupDlg : public StartupDlgBase
{
public:
    StartupDlg() = default;
    StartupDlg(wxWindow* parent, wxWindowID win_id = wxID_ANY,
               const wxString& title = "Open, Import, or Create Project",
               const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
               long style = wxDEFAULT_DIALOG_STYLE,
               const wxString& name =
                   wxDialogNameStr) :  // NOLINT (wxDialogNameStr issue) // cppcheck-suppress
        StartupDlgBase(parent, win_id, title, pos, size, style, name)
    {
    }

    enum class Command : std::uint8_t
    {
        start_mru,
        start_convert,
        start_open,
        start_empty,
    };

    [[nodiscard]] auto GetCommand() const -> Command { return m_command; }
    [[nodiscard]] auto GetProjectFile() -> wxFileName& { return m_value; }

protected:
    // Event handlers

    void OnImport(wxHyperlinkEvent& event) override;
    void OnInit(wxInitDialogEvent& event) override;
    void OnNew(wxHyperlinkEvent& event) override;
    void OnOpen(wxHyperlinkEvent& event) override;

    void OnHyperlink(wxHyperlinkEvent& event);
    void RemoveProjectFilename(wxCommandEvent& event);

private:
    wxFileName m_value;
    Command m_command { Command::start_empty };

    // Helper method to add projects to the grid, reduces code duplication
    void AddProjectToGrid(const wxString& display_name, const wxString& url,
                          const wxFileName& project_file, bool use_standard_colors);
};
