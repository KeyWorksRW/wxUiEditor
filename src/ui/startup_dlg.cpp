/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to display if wxUiEditor is launched with no arguments
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/display.h>   // wxDisplay
#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include "startup_dlg.h"  // #include "../wxui/startup_dlg_base.h"

#include "mainframe.h"  // Main frame
#include "version.h"    // Version numbers generated in ../CMakeLists.txt

// wxGenericHyperlinkCtrl has a DoContextMenu() method that displays "Copy URL" which isn't useful
// for StartDlg. What we need instead is an option to remove the project from the list.

class ttGenericHyperlinkCtrl : public wxGenericHyperlinkCtrl
{
public:
    ttGenericHyperlinkCtrl(wxWindow* parent, wxWindowID id, const wxString& label,
                           const wxString& url, const wxPoint& pos, const wxSize& size,
                           long style = wxHL_DEFAULT_STYLE) :
        wxGenericHyperlinkCtrl(parent, id, label, url, pos, size, style)
    {
        Bind(wxEVT_MENU, &ttGenericHyperlinkCtrl::RemoveProjectFilename, this, wxID_REMOVE);
    }

protected:
    void RemoveProjectFilename(wxCommandEvent& event)
    {
        event.SetString(GetURL());
        if (GetParent())
        {
            wxPostEvent(GetParent(), event);
        }
    }

    void DoContextMenu(const wxPoint& pos) override
    {
        wxMenu* menuPopUp = new wxMenu(wxEmptyString, wxMENU_TEAROFF);
        menuPopUp->Append(wxID_REMOVE, "Remove Project from List");
        PopupMenu(menuPopUp, pos);
        delete menuPopUp;
    }
};

void StartupDlg::OnInit(wxInitDialogEvent& event)
{
    if (!GetParent())
    {
        wxDisplay desktop(this);
        wxRect rect_parent(desktop.GetClientArea());
        wxRect rect_this(GetSize());
        rect_this.x = rect_parent.x + (rect_parent.width - rect_this.width) / 2;
        rect_this.y = rect_parent.y + (rect_parent.height - rect_this.height) / 3;
        SetSize(rect_this, wxSIZE_ALLOW_MINUS_ONE);
    }
    else
    {
        Center(wxHORIZONTAL);
    }

    m_name_version->SetLabel(txtVersion);

    auto& history = wxGetMainFrame()->getFileHistory();
    bool file_added = false;
    for (size_t idx = 0; idx < history.GetCount(); ++idx)
    {
        tt_string project_file = history.GetHistoryFile(idx).utf8_string();
        if (project_file.file_exists())
        {
            tt_string shortname = project_file.filename();
            shortname.remove_extension();
            project_file.remove_filename();

            auto hyperlink =
                new ttGenericHyperlinkCtrl(this, wxID_ANY, shortname, wxEmptyString,
                                           wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);

            wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
            font.SetSymbolicSize(wxFONTSIZE_LARGE);
            hyperlink->SetFont(font);
            // wxRED is the default for the generic version of this control, but we want to be
            // certain it stays that way in case wxWidgets changes. The colour change makes it
            // easier for the user to be certain they are over the link they want in the vertical
            // list of links.
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

    Bind(wxEVT_MENU, &StartupDlg::RemoveProjectFilename, this, wxID_REMOVE);

    if (wxGetApp().isTestingMenuEnabled())
    {
        auto append_history_ptr = wxGetFrame().GetAppendImportHistory();
        for (size_t idx = 0; idx < append_history_ptr->GetCount(); ++idx)
        {
            tt_string project_file = append_history_ptr->GetHistoryFile(idx).utf8_string();
            if (project_file.file_exists())
            {
                tt_string shortname = project_file.filename();
                project_file.remove_filename();

                // We do *not* use ttGenericHyperlinkCtrl here since that will remove the file from
                // the getFileHistory() list instead of GetAppendImportHistory().
                auto* hyperlink = new wxGenericHyperlinkCtrl(
                    this, wxID_ANY, shortname.make_wxString(), wxEmptyString, wxDefaultPosition,
                    wxDefaultSize, wxHL_DEFAULT_STYLE);

                wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
                font.SetSymbolicSize(wxFONTSIZE_LARGE);
                hyperlink->SetFont(font);
                // Reverse the colours so that it's obvious which ones are the Testing menu import
                // files.
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
    }

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

void StartupDlg::RemoveProjectFilename(wxCommandEvent& event)
{
    auto URL = event.GetString();
    auto& history = wxGetMainFrame()->getFileHistory();
    for (size_t idx = 0; idx < history.GetCount(); ++idx)
    {
        if (history.GetHistoryFile(idx) == URL)
        {
            history.RemoveFileFromHistory(idx);
            break;
        }
    }

    // Freeze the UI to prevent flicker during updates
    wxWindowUpdateLocker freeze(this);

    // Remove all children from the recent projects grid
    m_recent_flex_grid->Clear(true);
    for (size_t idx = 0; idx < history.GetCount(); ++idx)
    {
        tt_string project_file = history.GetHistoryFile(idx).utf8_string();
        if (project_file.file_exists())
        {
            tt_string shortname = project_file.filename();
            shortname.remove_extension();
            project_file.remove_filename();

            auto hyperlink =
                new ttGenericHyperlinkCtrl(this, wxID_ANY, shortname, wxEmptyString,
                                           wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);

            wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
            font.SetSymbolicSize(wxFONTSIZE_LARGE);
            hyperlink->SetFont(font);
            // wxRED is the default for the generic version of this control, but we want to be
            // certain it stays that way in case wxWidgets changes. The colour change makes it
            // easier for the user to be certain they are over the link they want in the vertical
            // list of links.
            hyperlink->SetHoverColour(*wxRED);
            hyperlink->SetURL(history.GetHistoryFile(idx));
            hyperlink->Bind(wxEVT_HYPERLINK, &StartupDlg::OnHyperlink, this);

            m_recent_flex_grid->Add(hyperlink, wxSizerFlags().Border(wxRIGHT));

            auto path = new wxStaticText(this, wxID_ANY, project_file);
            m_recent_flex_grid->Add(path, wxSizerFlags().Border(wxALL));
        }
    }

    if (wxGetApp().isTestingMenuEnabled())
    {
        auto append_history_ptr = wxGetFrame().GetAppendImportHistory();
        for (size_t idx = 0; idx < append_history_ptr->GetCount(); ++idx)
        {
            tt_string project_file = append_history_ptr->GetHistoryFile(idx).utf8_string();
            if (project_file.file_exists())
            {
                tt_string shortname = project_file.filename();
                project_file.remove_filename();

                auto* hyperlink = new ttGenericHyperlinkCtrl(
                    this, wxID_ANY, shortname.make_wxString(), wxEmptyString, wxDefaultPosition,
                    wxDefaultSize, wxHL_DEFAULT_STYLE);

                wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
                font.SetSymbolicSize(wxFONTSIZE_LARGE);
                hyperlink->SetFont(font);
                // Reverse the colours so that it's obvious which ones are the Debug build import
                // files.
                hyperlink->SetNormalColour(*wxRED);
                hyperlink->SetHoverColour(*wxBLUE);
                hyperlink->SetURL(append_history_ptr->GetHistoryFile(idx));
                hyperlink->Bind(wxEVT_HYPERLINK, &StartupDlg::OnHyperlink, this);

                m_recent_flex_grid->Add(hyperlink, wxSizerFlags().Border(wxRIGHT));

                auto path = new wxStaticText(this, wxID_ANY, project_file);
                m_recent_flex_grid->Add(path, wxSizerFlags().Border(wxALL));
            }
        }
    }

    Fit();
    Refresh();
}
