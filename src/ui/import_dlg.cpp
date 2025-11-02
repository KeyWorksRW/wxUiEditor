/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to import one or more projects
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>    // wxConfig base header
#include <wx/dir.h>       // wxDir is a class for enumerating the files in a directory
#include <wx/dirdlg.h>    // wxDirDialog base class
#include <wx/filedlg.h>   // wxFileDialog base header
#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include "import_dlg.h"  // auto-generated: import_base.h and import_base.cpp

#include "mainapp.h"         // App -- App class
#include "tt_view_vector.h"  // tt_view_vector -- read/write line-oriented strings/files
#include "ttwx.h"            // ttwx helpers for character classification

ImportDlg::ImportDlg(wxWindow* parent) : ImportBase(parent) {}

enum
{
    IMPORT_CRAFTER,
    IMPORT_DIALOGBLOCKS,
    IMPORT_FB,
    IMPORT_WINRES,
    IMPORT_GLADE,
    IMPORT_SMITH,
    IMPORT_XRC,
};

void ImportDlg::OnInitDialog(wxInitDialogEvent& /* event unused */)
{
    if (wxGetApp().isTestingMenuEnabled())
    {
        m_combo_recent_dirs->Show();
        m_btnRemove->Show();
    }

    m_stdBtn->GetAffirmativeButton()->Disable();
    m_radio_wxFormBuilder->SetFocus();

    auto config = wxConfig::Get();
    config->SetPath("/preferences");
    auto import_type = config->Read("import_type", IMPORT_FB);

    if (wxGetApp().isTestingMenuEnabled())
    {
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

            wxDir dir;
            wxArrayString files;

            m_checkListProjects->Clear();

            if (m_radio_wxCrafter->GetValue())
                dir.GetAllFiles(".", &files, "*.wxcp");
            else if (m_radio_wxFormBuilder->GetValue())
                dir.GetAllFiles(".", &files, "*.fbp");
            else if (m_radio_wxSmith->GetValue())
                dir.GetAllFiles(".", &files, "*.wxs");
            else if (m_radio_wxGlade->GetValue())
                dir.GetAllFiles(".", &files, "*.wxg");
            else if (m_radio_XRC->GetValue())
                dir.GetAllFiles(".", &files, "*.xrc");
            else if (m_radio_DialogBlocks->GetValue())
                dir.GetAllFiles(".", &files, "*.pjd");
            else if (m_radio_WindowsResource->GetValue())
            {
                dir.GetAllFiles(".", &files, "*.rc");
                dir.GetAllFiles(".", &files, "*.dlg");
                CheckResourceFiles(files);
            }

            if (files.size())
                m_checkListProjects->InsertItems(files, 0);
        }
    }

    config->SetPath("/");
    switch (import_type)
    {
        case IMPORT_CRAFTER:
            m_radio_wxCrafter->SetValue(true);
            break;

        case IMPORT_DIALOGBLOCKS:
            m_radio_DialogBlocks->SetValue(true);
            break;

        case IMPORT_WINRES:
            m_radio_WindowsResource->SetValue(true);
            m_staticImportList->SetLabel("&Files containing Dialogs or Menus:");
            break;

        case IMPORT_GLADE:
            m_radio_wxGlade->SetValue(true);
            break;

        case IMPORT_SMITH:
            m_radio_wxSmith->SetValue(true);
            break;

        case IMPORT_XRC:
            m_radio_XRC->SetValue(true);
            break;

        case IMPORT_FB:
            [[fallthrough]];
        default:
            m_radio_wxFormBuilder->SetValue(true);
            break;
    }

    wxCommandEvent dummy_event;

    OnRecentDir(dummy_event);

    if (wxGetApp().isTestingMenuEnabled())
    {
        // Because m_combo_recent_dirs was created hidden and is shown in Debug builds.
        Fit();
    }
}

void ImportDlg::OnCheckFiles(wxCommandEvent& /* event unused */)
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
            tt_string path = m_checkListProjects->GetString(pos).utf8_string();
            path.make_absolute();
            m_lstProjects.emplace_back(path);
        }
    }

    auto config = wxConfig::Get();
    config->SetPath("/preferences");
    if (m_radio_wxCrafter->GetValue())
        config->Write("import_type", static_cast<long>(IMPORT_CRAFTER));
    else if (m_radio_wxSmith->GetValue())
        config->Write("import_type", static_cast<long>(IMPORT_SMITH));
    else if (m_radio_wxGlade->GetValue())
        config->Write("import_type", static_cast<long>(IMPORT_GLADE));
    else if (m_radio_XRC->GetValue())
        config->Write("import_type", static_cast<long>(IMPORT_XRC));
    else if (m_radio_WindowsResource->GetValue())
        config->Write("import_type", static_cast<long>(IMPORT_WINRES));
    else if (m_radio_DialogBlocks->GetValue())
        config->Write("import_type", static_cast<long>(IMPORT_DIALOGBLOCKS));
    else
        config->Write("import_type", static_cast<long>(IMPORT_FB));

    if (wxGetApp().isTestingMenuEnabled())
    {
        m_FileHistory.Save(*config);
    }
    config->SetPath("/");

    event.Skip();
}

void ImportDlg::OnDirectory(wxCommandEvent& /* event unused */)
{
    wxDirDialog dlg(this, "Choose directory", wxEmptyString,
                    wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    dlg.SetPath(m_static_cwd->GetLabel());
    if (dlg.ShowModal() != wxID_OK)
        return;

    if (wxGetApp().isTestingMenuEnabled())
    {
        m_FileHistory.AddFileToHistory(dlg.GetPath());
        m_combo_recent_dirs->AppendString(dlg.GetPath());
    }

    tt::ChangeDir(dlg.GetPath().utf8_string());

    tt_string cwd;
    cwd.assignCwd();
    m_static_cwd->SetLabel(cwd.make_wxString());

    wxDir dir;
    wxArrayString files;

    m_checkListProjects->Clear();

    wxBusyCursor wait;

    if (m_radio_wxCrafter->GetValue())
        dir.GetAllFiles(".", &files, "*.wxcp");
    else if (m_radio_wxFormBuilder->GetValue())
        dir.GetAllFiles(".", &files, "*.fbp");
    else if (m_radio_wxSmith->GetValue())
        dir.GetAllFiles(".", &files, "*.wxs");
    else if (m_radio_wxGlade->GetValue())
        dir.GetAllFiles(".", &files, "*.wxg");
    else if (m_radio_XRC->GetValue())
        dir.GetAllFiles(".", &files, "*.xrc");
    else if (m_radio_DialogBlocks->GetValue())
        dir.GetAllFiles(".", &files, "*.pjd");
    else if (m_radio_WindowsResource->GetValue())
    {
        dir.GetAllFiles(".", &files, "*.rc");
        dir.GetAllFiles(".", &files, "*.dlg");
        CheckResourceFiles(files);
    }

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnRecentDir(wxCommandEvent& /* event unused */)
{
    if (wxGetApp().isTestingMenuEnabled())
    {
        auto result = m_combo_recent_dirs->GetValue();
        m_FileHistory.AddFileToHistory(result);
        tt::ChangeDir(result.utf8_string());

        tt_string cwd;
        cwd.assignCwd();
        m_static_cwd->SetLabel(cwd.make_wxString());

        wxDir dir;
        wxArrayString files;

        m_checkListProjects->Clear();

        wxBusyCursor wait;

        if (m_radio_wxCrafter->GetValue())
            dir.GetAllFiles(".", &files, "*.wxcp");
        else if (m_radio_wxFormBuilder->GetValue())
            dir.GetAllFiles(".", &files, "*.fbp");
        else if (m_radio_wxSmith->GetValue())
            dir.GetAllFiles(".", &files, "*.wxs");
        else if (m_radio_wxGlade->GetValue())
            dir.GetAllFiles(".", &files, "*.wxg");
        else if (m_radio_XRC->GetValue())
            dir.GetAllFiles(".", &files, "*.xrc");
        else if (m_radio_DialogBlocks->GetValue())
            dir.GetAllFiles(".", &files, "*.pjd");
        else if (m_radio_WindowsResource->GetValue())
        {
            dir.GetAllFiles(".", &files, "*.rc");
            dir.GetAllFiles(".", &files, "*.dlg");
            CheckResourceFiles(files);
        }

        if (files.size())
            m_checkListProjects->InsertItems(files, 0);
    }
}

void ImportDlg::OnRemove(wxCommandEvent& event)
{
    if (wxGetApp().isTestingMenuEnabled())
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
}

void ImportDlg::OnCrafter(wxCommandEvent& /* event unused */)
{
    m_checkListProjects->Clear();
    m_staticImportList->SetLabel("&Files:");

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.wxcp");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnFormBuilder(wxCommandEvent& /* event unused */)
{
    m_checkListProjects->Clear();
    m_staticImportList->SetLabel("&Files:");

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.fbp");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnDialogBlocks(wxCommandEvent& /* event unused */)
{
    m_checkListProjects->Clear();
    m_staticImportList->SetLabel("&Files:");

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.pjd");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnWindowsResource(wxCommandEvent& /* event unused */)
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

void ImportDlg::OnWxSmith(wxCommandEvent& /* event unused */)
{
    m_checkListProjects->Clear();
    m_staticImportList->SetLabel("&Files:");

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.wxs");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnXRC(wxCommandEvent& /* event unused */)
{
    m_checkListProjects->Clear();

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.xrc");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnWxGlade(wxCommandEvent& /* event unused */)
{
    m_checkListProjects->Clear();
    m_staticImportList->SetLabel("&Files:");

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.wxg");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnSelectAll(wxCommandEvent& /* event unused */)
{
    for (unsigned int pos = 0; pos < m_checkListProjects->GetCount(); ++pos)
    {
        m_checkListProjects->Check(pos, true);
    }
    m_stdBtn->GetAffirmativeButton()->Enable();
}

void ImportDlg::OnSelectNone(wxCommandEvent& /* event unused */)
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

    tt_view_vector rc_file;

    for (size_t idx = 0; idx < files.size(); ++idx)
    {
        bool found = false;
        if (rc_file.ReadFile(files[idx].utf8_string()))
        {
            for (auto& line: rc_file)
            {
                if (line.empty() || !ttwx::is_alpha(line[0]))
                    continue;

                auto type = line.view_stepover();

                // If there is a DESIGNINFO section, there may be a DIALOG specified for APSTUDIO to
                // used -- however that dialog may not actually exist. So instead, we look for a
                // trailing space which should indicate the statement is followed by dimensions.

                if (type.starts_with("DIALOG ") || type.starts_with("DIALOGEX ") ||
                    type.starts_with("MENU"))
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
