/////////////////////////////////////////////////////////////////////////////
// Purpose:   Convert one or more Windows Resource files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// CAUTION! This dialog is only used in Debug builds. It displayes a recently used list of folders containing resource files,
// as well as displaying a list of all resource files within a selected folder. That second list contains checkboxes, so
// multiple files can be selected as desired. The OK button then converts the requested resource file(s) but does *NOT* save
// the current project! The idea is to quickly convert multiple resource files to verify changes in the conversio code.

#include "pch.h"

#include <wx/arrstr.h>       // wxArrayString class
#include <wx/config.h>       // wxConfig base header
#include <wx/dir.h>          // wxDir is a class for enumerating the files in a directory
#include <wx/dirdlg.h>       // wxDirDialog base class
#include <wx/filehistory.h>  // wxFileHistory class

#include "dbg_winres_dlg.h"  // auto-generated: dbg_winres_dlg_base.h and dbg_winres_dlg_base.cpp

inline constexpr auto txt_dbg_winres_history = "/winres";

DbgWinResDlg::DbgWinResDlg(wxWindow* parent) : DbgWinResBase(parent) {}

void DbgWinResDlg::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    auto config = wxConfig::Get();
    config->SetPath(txt_dbg_winres_history);

    m_history.Load(*config);
    config->SetPath("/");

    for (size_t idx = 0; idx < m_history.GetCount(); ++idx)
    {
        m_list_folders->Append(m_history.GetHistoryFile(idx));
    }

    if (m_history.GetCount() > 0)
    {
        m_list_folders->Select(0);
        wxCommandEvent event;
        OnSelectFolder(event);
    }

    m_list_folders->SetFocus();
}

void DbgWinResDlg::OnSelectFolder(wxCommandEvent& WXUNUSED(event))
{
    auto cur_selection = m_list_folders->GetSelection();
    if (cur_selection >= 0)
    {
        auto folder = m_list_folders->GetString(cur_selection);
        wxDir dir;
        wxArrayString files;
        dir.GetAllFiles(folder, &files, "*.rc");
        dir.GetAllFiles(folder, &files, "*.dlg");

        m_list_files->Clear();

        for (const auto& file: files)
        {
            m_list_files->Append(file);
        }

        if (files.GetCount())
        {
            m_list_files->Select(0);
            m_res_file->SetValue(m_list_files->GetString(m_list_files->GetSelection()));
        }
    }
}

void DbgWinResDlg::OnFolderBtn(wxCommandEvent& WXUNUSED(event))
{
    wxDirDialog dlg(this, "Choose directory", wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() != wxID_OK)
        return;

    auto path = dlg.GetPath();
    auto sel = m_list_folders->Append(path);
    m_list_folders->Select(sel);
    wxCommandEvent event;
    OnSelectFolder(event);

    m_history.AddFileToHistory(path);
    auto config = wxConfig::Get();
    config->SetPath(txt_dbg_winres_history);
    m_history.Save(*config);
    config->SetPath("/");
}

void DbgWinResDlg::OnAffirmative(wxCommandEvent& event)
{
    auto cur_selection = m_list_files->GetSelection();
    if (cur_selection >= 0)
    {
        m_filename = m_list_files->GetString(cur_selection);
    }

    event.Skip();
}
