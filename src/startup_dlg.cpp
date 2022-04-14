/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to display if wxUiEditor is launched with no arguments
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "ui/startupdlg_base.h"  // auto-generated: ui/startupdlg_base.h and ui/startupdlg_base.cpp

#include "mainapp.h"    // App -- Main application class
#include "mainframe.h"  // MainFrame -- Main window frame

// If this constructor is used, the caller must call Create(parent)

void StartupDlg::OnInit(wxInitDialogEvent& event)
{
    m_name_version->SetLabel(txtVersion);

    auto& history = wxGetApp().GetMainFrame()->GetFileHistory();
    bool file_added = false;
    for (size_t idx = 0; idx < history.GetCount(); ++idx)
    {
        ttString project_file = history.GetHistoryFile(idx);
        if (project_file.file_exists())
        {
            auto shortname = project_file.filename();
            shortname.remove_extension();
            project_file.remove_filename();

            auto hyperlink = new wxGenericHyperlinkCtrl(this, wxID_ANY, shortname, wxEmptyString, wxDefaultPosition,
                                                        wxDefaultSize, wxHL_DEFAULT_STYLE);

            wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
            font.SetSymbolicSize(wxFONTSIZE_LARGE);
            hyperlink->SetFont(font);
            // wxRED is the default for the generic version of this control, but we want to be certain it stays that way in
            // case wxWidgets changes. The colour change makes it easier for the user to be certain they are over the link
            // they want in the vertical list of links.
            hyperlink->SetHoverColour(*wxRED);
            hyperlink->SetURL(history.GetHistoryFile(idx));
            hyperlink->Bind(wxEVT_HYPERLINK, &StartupDlg::OnHyperlink, this);

            m_recent_flex_grid->Add(hyperlink, wxSizerFlags().Border(wxRIGHT));

            auto path = new wxStaticText(this, wxID_ANY, project_file);
            m_recent_flex_grid->Add(path, wxSizerFlags().Border(wxALL));
            file_added = true;
        }
        else
        {
            // Assume that if the file doesn't exist now, it won't exist later either
            history.RemoveFileFromHistory(idx--);
        }
    }

#if defined(_DEBUG) || defined(INTERNAL_WIDGETS)
    auto append_history_ptr = wxGetFrame().GetAppendImportHistory();
    for (size_t idx = 0; idx < append_history_ptr->GetCount(); ++idx)
    {
        ttString project_file = append_history_ptr->GetHistoryFile(idx);
        if (project_file.file_exists())
        {
            auto shortname = project_file.filename();
            project_file.remove_filename();

            auto hyperlink = new wxGenericHyperlinkCtrl(this, wxID_ANY, shortname, wxEmptyString, wxDefaultPosition,
                                                        wxDefaultSize, wxHL_DEFAULT_STYLE);

            wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
            font.SetSymbolicSize(wxFONTSIZE_LARGE);
            hyperlink->SetFont(font);
            // Reverse the colours so that it's obvious which ones are the Debug build import files.
            hyperlink->SetNormalColour(*wxRED);
            hyperlink->SetHoverColour(*wxBLUE);
            hyperlink->SetURL(append_history_ptr->GetHistoryFile(idx));
            hyperlink->Bind(wxEVT_HYPERLINK, &StartupDlg::OnHyperlink, this);

            m_recent_flex_grid->Add(hyperlink, wxSizerFlags().Border(wxRIGHT));

            auto path = new wxStaticText(this, wxID_ANY, project_file);
            m_recent_flex_grid->Add(path, wxSizerFlags().Border(wxALL));
            file_added = true;
        }
    }
#endif

    if (file_added)
    {
        m_staticTextRecentProjects->Hide();
    }

    Fit();

    event.Skip();  // transfer all validator data to their windows and update UI
}

void StartupDlg::OnHyperlink(wxHyperlinkEvent& event)
{
    m_cmdType = START_MRU;
    m_value = event.GetURL();
    EndModal(wxID_OK);
}

void StartupDlg::OnImport(wxHyperlinkEvent& WXUNUSED(event))
{
    m_cmdType = START_CONVERT;
    EndModal(wxID_OK);
}

void StartupDlg::OnOpen(wxHyperlinkEvent& WXUNUSED(event))
{
    m_cmdType = START_OPEN;
    EndModal(wxID_OK);
}

void StartupDlg::OnNew(wxHyperlinkEvent& WXUNUSED(event))
{
    m_cmdType = START_EMPTY;
    EndModal(wxID_OK);
}
