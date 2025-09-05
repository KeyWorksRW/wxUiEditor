/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to display if wxUiEditor is launched with no arguments
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../wxui/startup_dlg_base.h"

class StartupDlg : public StartupDlgBase
{
public:
    StartupDlg() : StartupDlgBase() {}
    StartupDlg(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxString& title = "Open, Import, or Create Project",
               const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
               long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxDialogNameStr) :
        StartupDlgBase(parent, id, title, pos, size, style, name)
    {
    }

    enum
    {
        START_MRU,
        START_CONVERT,
        START_OPEN,
        START_EMPTY,
    };

    auto GetCommandType() const { return m_cmdType; }
    tt_string& GetProjectFile() { return m_value; }

protected:
    // Event handlers

    void OnImport(wxHyperlinkEvent& event) override;
    void OnInit(wxInitDialogEvent& event) override;
    void OnNew(wxHyperlinkEvent& event) override;
    void OnOpen(wxHyperlinkEvent& event) override;

    void OnHyperlink(wxHyperlinkEvent& event);
    void RemoveProjectFilename(wxCommandEvent& event);

private:
    tt_string m_value;
    size_t m_cmdType { START_EMPTY };
};
