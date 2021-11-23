/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to import one or more projects
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>   // wxConfig base header
#include <wx/dir.h>      // wxDir is a class for enumerating the files in a directory
#include <wx/dirdlg.h>   // wxDirDialog base class
#include <wx/filedlg.h>  // wxFileDialog base header

#include "pugixml.hpp"   // xml processing
#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "import_dlg.h"  // auto-generated: import_base.h and import_base.cpp

ImportDlg::ImportDlg(wxWindow* parent) : ImportBase(parent) {}

enum
{
    DBG_IMPORT_FB,
    DBG_IMPORT_WINRES,
    DBG_IMPORT_GLADE,
    DBG_IMPORT_SMITH,
    DBG_IMPORT_XRC,
};

void ImportDlg::OnInitDialog(wxInitDialogEvent& WXUNUSED(event))
{
#if defined(_DEBUG)
    m_combo_recent_dirs->Show();
    m_btnRemove->Show();
#endif  // _DEBUG

    m_stdBtn->GetAffirmativeButton()->Disable();
    m_radio_wxFormBuilder->SetFocus();

    ttString cwd;
    cwd.assignCwd();
    m_static_cwd->SetLabel(cwd);

    auto config = wxConfig::Get();
    config->SetPath("/preferences");
    auto import_type = config->Read("import_type", DBG_IMPORT_FB);

#if defined(_DEBUG)
    m_FileHistory.Load(*config);
    for (size_t idx = 0; idx < m_FileHistory.GetCount(); ++idx)
    {
        m_combo_recent_dirs->AppendString(m_FileHistory.GetHistoryFile(idx));
    }
    if (m_FileHistory.GetCount())
    {
        m_combo_recent_dirs->Select(0);
        wxFileName::SetCwd(m_combo_recent_dirs->GetValue());
        m_static_cwd->SetLabel(m_combo_recent_dirs->GetValue());
    }
#endif  // _DEBUG

    config->SetPath("/");
    switch (import_type)
    {
        case DBG_IMPORT_WINRES:
            m_radio_WindowsResource->SetValue(true);
            m_staticImportList->SetLabel("&Files containing Dialogs or Menus:");
            break;

        case DBG_IMPORT_GLADE:
            m_radio_wxGlade->SetValue(true);
            break;

        case DBG_IMPORT_SMITH:
            m_radio_wxSmith->SetValue(true);
            break;

        case DBG_IMPORT_XRC:
            m_radio_XRC->SetValue(true);
            break;

        case DBG_IMPORT_FB:
            [[fallthrough]];
        default:
            m_radio_wxFormBuilder->SetValue(true);
            break;
    }

    wxDir dir;
    wxArrayString files;

    m_checkListProjects->Clear();

    if (m_radio_wxFormBuilder->GetValue())
        dir.GetAllFiles(".", &files, "*.fbp");
    else if (m_radio_wxSmith->GetValue())
        dir.GetAllFiles(".", &files, "*.wxs");
    else if (m_radio_wxGlade->GetValue())
        dir.GetAllFiles(".", &files, "*.wxg");
    else if (m_radio_XRC->GetValue())
        dir.GetAllFiles(".", &files, "*.xrc");
    else if (m_radio_WindowsResource->GetValue())
    {
        dir.GetAllFiles(".", &files, "*.rc");
        dir.GetAllFiles(".", &files, "*.dlg");
        CheckResourceFiles(files);
    }

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);

#if defined(_DEBUG)
    // Because m_combo_recent_dirs was created hidden and is shown in Debug builds.
    Fit();
#endif
}

void ImportDlg::OnCheckFiles(wxCommandEvent& WXUNUSED(event))
{
    m_stdBtn->GetAffirmativeButton()->Disable();

    for (unsigned int pos = 0; pos < m_checkListProjects->GetCount(); ++pos)
    {
        if (m_checkListProjects->IsChecked(pos))
        {
            m_stdBtn->GetAffirmativeButton()->Enable();
            return;
        }
    }
}

void ImportDlg::OnOK(wxCommandEvent& event)
{
    for (unsigned int pos = 0; pos < m_checkListProjects->GetCount(); ++pos)
    {
        if (m_checkListProjects->IsChecked(pos))
        {
            m_lstProjects.emplace_back(m_checkListProjects->GetString(pos));
        }
    }

    auto config = wxConfig::Get();
    config->SetPath("/preferences");
    if (m_radio_wxSmith->GetValue())
        config->Write("import_type", static_cast<long>(DBG_IMPORT_SMITH));
    else if (m_radio_wxGlade->GetValue())
        config->Write("import_type", static_cast<long>(DBG_IMPORT_GLADE));
    else if (m_radio_XRC->GetValue())
        config->Write("import_type", static_cast<long>(DBG_IMPORT_XRC));
    else if (m_radio_WindowsResource->GetValue())
        config->Write("import_type", static_cast<long>(DBG_IMPORT_WINRES));
    else
        config->Write("import_type", static_cast<long>(DBG_IMPORT_FB));

#if defined(_DEBUG)
    m_FileHistory.Save(*config);
#endif
    config->SetPath("/");

    event.Skip();
}

void ImportDlg::OnDirectory(wxCommandEvent& WXUNUSED(event))
{
    wxDirDialog dlg(this, "Choose directory", wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    dlg.SetPath(m_static_cwd->GetLabel());
    if (dlg.ShowModal() != wxID_OK)
        return;

#if defined(_DEBUG)
    m_FileHistory.AddFileToHistory(dlg.GetPath());
    m_combo_recent_dirs->AppendString(dlg.GetPath());
#endif

    ttlib::ChangeDir(dlg.GetPath().utf8_str().data());

    ttString cwd;
    cwd.assignCwd();
    m_static_cwd->SetLabel(cwd);

    wxDir dir;
    wxArrayString files;

    m_checkListProjects->Clear();

    wxBusyCursor wait;

    if (m_radio_wxFormBuilder->GetValue())
        dir.GetAllFiles(".", &files, "*.fbp");
    else if (m_radio_wxSmith->GetValue())
        dir.GetAllFiles(".", &files, "*.wxs");
    else if (m_radio_wxGlade->GetValue())
        dir.GetAllFiles(".", &files, "*.wxg");
    else if (m_radio_XRC->GetValue())
        dir.GetAllFiles(".", &files, "*.xrc");
    else if (m_radio_WindowsResource->GetValue())
    {
        dir.GetAllFiles(".", &files, "*.rc");
        dir.GetAllFiles(".", &files, "*.dlg");
        CheckResourceFiles(files);
    }

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

#if defined(_DEBUG)
void ImportDlg::OnRecentDir(wxCommandEvent& WXUNUSED(event))
{
    auto result = m_combo_recent_dirs->GetValue();
    ttlib::ChangeDir(result.utf8_str().data());

    ttString cwd;
    cwd.assignCwd();
    m_static_cwd->SetLabel(cwd);

    wxDir dir;
    wxArrayString files;

    m_checkListProjects->Clear();

    wxBusyCursor wait;

    if (m_radio_wxFormBuilder->GetValue())
        dir.GetAllFiles(".", &files, "*.fbp");
    else if (m_radio_wxSmith->GetValue())
        dir.GetAllFiles(".", &files, "*.wxs");
    else if (m_radio_wxGlade->GetValue())
        dir.GetAllFiles(".", &files, "*.wxg");
    else if (m_radio_XRC->GetValue())
        dir.GetAllFiles(".", &files, "*.xrc");
    else if (m_radio_WindowsResource->GetValue())
    {
        dir.GetAllFiles(".", &files, "*.rc");
        dir.GetAllFiles(".", &files, "*.dlg");
        CheckResourceFiles(files);
    }

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnRemove(wxCommandEvent& event)
{
    auto directory = m_combo_recent_dirs->GetValue();
    for (size_t idx = 0; idx < m_FileHistory.GetCount(); ++idx)
    {
        if (m_FileHistory.GetHistoryFile(idx) == directory)
        {
            m_FileHistory.RemoveFileFromHistory(idx);
            auto config = wxConfig::Get();
            config->SetPath("/preferences");
            m_FileHistory.Save(*config);
            config->SetPath("/");

            m_combo_recent_dirs->Clear();
            for (idx = 0; idx < m_FileHistory.GetCount(); ++idx)
            {
                m_combo_recent_dirs->AppendString(m_FileHistory.GetHistoryFile(idx));
            }
            if (m_FileHistory.GetCount())
            {
                m_combo_recent_dirs->Select(0);
                OnRecentDir(event);
            }
            break;
        }
    }
}

#endif  // _DEBUG

void ImportDlg::OnFormBuilder(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();
    m_staticImportList->SetLabel("&Files:");

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.fbp");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnWindowsResource(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();
    m_staticImportList->SetLabel("&Files containing Dialogs or Menus:");

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.rc");
    dir.GetAllFiles(".", &files, "*.dlg");
    CheckResourceFiles(files);

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnWxSmith(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();
    m_staticImportList->SetLabel("&Files:");

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.wxs");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnXRC(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.xrc");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnWxGlade(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();
    m_staticImportList->SetLabel("&Files:");

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.wxg");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnSelectAll(wxCommandEvent& WXUNUSED(event))
{
    for (unsigned int pos = 0; pos < m_checkListProjects->GetCount(); ++pos)
    {
        m_checkListProjects->Check(pos, true);
    }
    m_stdBtn->GetAffirmativeButton()->Enable();
}

void ImportDlg::OnSelectNone(wxCommandEvent& WXUNUSED(event))
{
    for (unsigned int pos = 0; pos < m_checkListProjects->GetCount(); ++pos)
    {
        m_checkListProjects->Check(pos, false);
    }
    m_stdBtn->GetAffirmativeButton()->Disable();
}

void ImportDlg::CheckResourceFiles(wxArrayString& files)
{
    wxBusyCursor busy;

    ttlib::viewfile rc_file;

    for (size_t idx = 0; idx < files.size(); ++idx)
    {
        bool found = false;
        if (rc_file.ReadFile(files[idx].utf8_string()))
        {
            for (auto& line: rc_file)
            {
                if (line.empty() || !ttlib::is_alpha(line[0]))
                    continue;

                auto type = line.view_stepover();

                // If there is a DESIGNINFO section, there may be a DIALOG specified for APSTUDIO to used -- however that
                // dialog may not actually exist. So instead, we look for a trailing space which should indicate the
                // statement is followed by dimensions.

                if (type.is_sameprefix("DIALOG ") || type.is_sameprefix("DIALOGEX ") || type.is_sameprefix("MENU"))
                {
                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            files.RemoveAt(idx);
            --idx;  // because the for() loop will increment this
        }
    }
}
