/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to display if wxUiEditor is launched with no arguments
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/display.h>   // wxDisplay
#include <wx/filedlg.h>   // wxFileDialog base header
#include <wx/filename.h>  // wxFileName class
#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include <format>

#include "startup_dlg.h"  // #include "../wxui/startup_dlg_base.h"

#include "mainframe.h"        // Main frame
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties
#include "version.h"          // Version numbers generated in ../CMakeLists.txt

// wxGenericHyperlinkCtrl has a DoContextMenu() method that displays "Copy URL" which isn't useful
// for StartDlg. What we need instead is an option to remove the project from the list.

class RemovableProjectHyperlinkCtrl : public wxGenericHyperlinkCtrl
{
public:
    RemovableProjectHyperlinkCtrl(wxWindow* parent, wxWindowID win_id, const wxString& label,
                                  const wxString& url, const wxPoint& pos, const wxSize& size,
                                  long style = wxHL_DEFAULT_STYLE) :
        wxGenericHyperlinkCtrl(parent, win_id, label, url, pos, size, style)
    {
        Bind(wxEVT_MENU, &RemovableProjectHyperlinkCtrl::RemoveProjectFilename, this, wxID_REMOVE);
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
        auto menu_popup = std::make_unique<wxMenu>(wxEmptyString, wxMENU_TEAROFF);
        menu_popup->Append(wxID_REMOVE, "Remove Project from List");
        PopupMenu(menu_popup.get(), pos);
    }
};

// Adds a project hyperlink and path to the grid
// use_standard_colors: true for main history (uses RemovableProjectHyperlinkCtrl with remove
// option),
//                      false for testing imports (uses wxGenericHyperlinkCtrl with inverted colors)
void StartupDlg::AddProjectToGrid(const wxString& display_name, const wxString& url,
                                  const wxFileName& project_file, bool use_standard_colors)
{
    wxGenericHyperlinkCtrl* hyperlink = nullptr;
    if (use_standard_colors)
    {
        hyperlink =
            new RemovableProjectHyperlinkCtrl(this, wxID_ANY, display_name, wxEmptyString,
                                              wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
    }
    else
    {
        hyperlink =
            new wxGenericHyperlinkCtrl(this, wxID_ANY, display_name, wxEmptyString,
                                       wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
    }

    wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
    font.SetSymbolicSize(wxFONTSIZE_LARGE);
    hyperlink->SetFont(font);

    if (use_standard_colors)
    {
        hyperlink->SetHoverColour(*wxRED);
    }
    else
    {
        hyperlink->SetNormalColour(*wxRED);
        hyperlink->SetHoverColour(*wxBLUE);
    }

    hyperlink->SetURL(url);
    hyperlink->Bind(wxEVT_HYPERLINK, &StartupDlg::OnHyperlink, this);

    m_recent_flex_grid->Add(hyperlink, wxSizerFlags().Border(wxRIGHT));

    auto* path = new wxStaticText(this, wxID_ANY, project_file.GetPath());
    m_recent_flex_grid->Add(path, wxSizerFlags().Border(wxALL));
}

void StartupDlg::OnInit(wxInitDialogEvent& event)
{
    if (!GetParent())
    {
        wxDisplay desktop(this);
        wxRect rect_parent(desktop.GetClientArea());
        wxRect rect_this(GetSize());
        rect_this.x = rect_parent.x + ((rect_parent.width - rect_this.width) / 2);
        rect_this.y = rect_parent.y + ((rect_parent.height - rect_this.height) / 3);
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
        wxString history_file = history.GetHistoryFile(idx);
        wxFileName project_file(history_file);
        if (project_file.FileExists())
        {
            wxFileName shortname = project_file;
            shortname.SetExt(wxEmptyString);

            AddProjectToGrid(shortname.GetName(), history_file, project_file, true);
            file_added = true;
        }
        else
        {
            // Assume that if the file doesn't exist now, it won't exist later either
            history.RemoveFileFromHistory(idx);
            --idx;  // Adjust index after removal
        }
    }

    Bind(wxEVT_MENU, &StartupDlg::RemoveProjectFilename, this, wxID_REMOVE);

    if (wxGetApp().isTestingMenuEnabled())
    {
        auto* append_history_ptr = wxGetFrame().GetAppendImportHistory();
        for (size_t idx = 0; idx < append_history_ptr->GetCount(); ++idx)
        {
            wxString history_file = append_history_ptr->GetHistoryFile(idx);
            wxFileName project_file(history_file);
            if (project_file.FileExists())
            {
                // We do *not* use RemovableProjectHyperlinkCtrl here since that will remove the
                // file from the getFileHistory() list instead of GetAppendImportHistory().
                AddProjectToGrid(project_file.GetName(), history_file, project_file, false);
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
    m_command = Command::start_mru;
    m_value = event.GetURL();
    EndModal(wxID_OK);
}

void StartupDlg::OnImport(wxHyperlinkEvent& /* event unused */)
{
    m_command = Command::start_convert;
    EndModal(wxID_OK);
}

void StartupDlg::OnOpen(wxHyperlinkEvent& /* event unused */)
{
    m_command = Command::start_open;
    EndModal(wxID_OK);
}

void StartupDlg::OnNew(wxHyperlinkEvent& /* event unused */)
{
    m_command = Command::start_empty;
    EndModal(wxID_OK);
}

void StartupDlg::RemoveProjectFilename(wxCommandEvent& event)
{
    auto url = event.GetString();
    auto& history = wxGetMainFrame()->getFileHistory();
    for (size_t idx = 0; idx < history.GetCount(); ++idx)
    {
        if (history.GetHistoryFile(idx) == url)
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
        wxString history_file = history.GetHistoryFile(idx);
        wxFileName project_file(history_file);
        if (project_file.FileExists())
        {
            wxFileName shortname = project_file;
            shortname.SetExt(wxEmptyString);

            AddProjectToGrid(shortname.GetName(), history_file, project_file, true);
        }
    }

    if (wxGetApp().isTestingMenuEnabled())
    {
        auto* append_history_ptr = wxGetFrame().GetAppendImportHistory();
        for (size_t idx = 0; idx < append_history_ptr->GetCount(); ++idx)
        {
            wxString history_file = append_history_ptr->GetHistoryFile(idx);
            wxFileName project_file(history_file);
            if (project_file.FileExists())
            {
                // We do *not* use RemovableProjectHyperlinkCtrl here since that will remove the
                // file from the getFileHistory() list instead of GetAppendImportHistory().
                AddProjectToGrid(project_file.GetName(), history_file, project_file, false);
            }
        }
    }

    Fit();
    Refresh();
}

auto DsisplayStartupDlg(wxWindow* parent) -> bool
{
    StartupDlg start_dlg(parent);
    if (auto result = start_dlg.ShowModal(); result == wxID_OK)
    {
        switch (start_dlg.GetCommand())
        {
            case StartupDlg::Command::start_mru:
                {
                    auto& project_file = start_dlg.GetProjectFile();
                    auto ext = project_file.GetExt().Lower().ToStdString();
                    ext.insert(ext.begin(), '.');

                    if (ext != PROJECT_FILE_EXTENSION && ext != PROJECT_LEGACY_FILE_EXTENSION)
                    {
                        return Project.ImportProject(project_file.GetFullPath().ToStdString());
                    }
                    return Project.LoadProject(project_file.GetFullPath().ToStdString());
                }
                break;

            case StartupDlg::Command::start_empty:
                return Project.NewProject(true);
                break;

            case StartupDlg::Command::start_convert:
                return Project.NewProject(false);
                break;

            case StartupDlg::Command::start_open:
                {
                    // TODO: [KeyWorks - 02-21-2021] A CodeBlocks file will contain all of the
                    // wxSmith resources -- so it would actually make sense to process it since
                    // we can combine all of those resources into our single project file.

                    auto path = ShowOpenProjectDialog(nullptr);
                    if (!path.IsEmpty())
                    {
                        tt_string filename = path.utf8_string();
                        if (!filename.extension().is_sameas(PROJECT_FILE_EXTENSION,
                                                            tt::CASE::either) &&
                            !filename.extension().is_sameas(PROJECT_LEGACY_FILE_EXTENSION,
                                                            tt::CASE::either))
                        {
                            return Project.ImportProject(filename);
                        }
                        return Project.LoadProject(path);
                    }
                }
        }
    }
    return false;
}
